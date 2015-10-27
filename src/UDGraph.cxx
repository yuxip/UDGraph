/*! \brief UUID-Domain Graph.
 *
 * Created on: Oct. 24, 2015
 * Author: Yuxi Pan
 *
 * implement adjacency list for graph structure
 * and depth first / breath first traversal
 */

#include<cstdlib>
#include<cstring>
#include<cmath>
#include<fstream>
#include<algorithm>

#include "UDGraph.h"

void ArcData::Print() {
	std::cout << " Date: " << mDate << std::endl;
	std::cout << "t_diff: " << mTdiff << std::endl;
	std::cout << "---------------SHA list: " << std::endl;
	for (std::vector<std::string>::iterator it = mSha_list.begin();
			it != mSha_list.end(); it++)
		std::cout << (*it) << std::endl;

}

UNode::UNode(std::string uuid, std::string guid, ArcNode* firstarc) :
		mUuid(uuid), mGuid(guid), mFirstarc(firstarc) {
}

UNode::UNode(const UNode& other) {
	mUuid = other.mUuid;
	mGuid = other.mGuid;
	mFirstarc = other.mFirstarc;
}

void UNode::PrintData() {
	std::cout << "UUID: " << mUuid << "GUID: " << mGuid << std::endl;
}

DNode::DNode(std::string domain, std::string ip, std::string date,
		bool reptool_tag, float pre_score, float post_score) :
		mDomain(domain), mIp(ip), mDate(date), mReptool_tag(reptool_tag), mPre_score_sum(
				pre_score), mPost_score_sum(post_score) {
}

DNode::DNode(const DNode& other) {

	mDomain = other.mDomain;
	mIp = other.mIp;
	mDate = other.mDate;
	mReptool_tag = other.mReptool_tag;
	mPre_score_sum = other.mPre_score_sum;
	mPost_score_sum = other.mPost_score_sum;
}

void DNode::PrintData() {

	std::cout << "-----------------Domain-IP node: " << std::endl;
	std::cout << "Domain: " << mDomain << std::endl;
	std::cout << "IP: " << mIp << std::endl;
	std::cout << "Date scores are computed: " << mDate << std::endl;
	std::cout << "Reptool_tag: " << mReptool_tag << std::endl;
	std::cout << "Pre_score_sum: " << mPre_score_sum << std::endl;
	std::cout << "Post_score_sum: " << mPost_score_sum << std::endl;

}

//! created UDGraph from batch-processed results.
/*!
 *
 * @param infection_file contains the domains contacted in each infection instances
 * format of infection_file:
 * time_difference, domain name, uuid, date, [list of malicious SHAs involved]
 * @return
 */
int UDGraph::createUDGraph(const char* infection_file, const char* ref_date) {

	std::ifstream infile(infection_file, std::ios::in);
	std::cout << "opening file: " << infection_file << std::endl;
	if (!infile) {
		std::cout << "ERROR opening file " << infection_file << std::endl;
		return -1;
	}
	int tdiff;
	char uuid[200];
	char sha_list[1000];
	char date[30];
	char domain[500];
	for (std::string line; std::getline(infile, line);) {

		sscanf(line.c_str(), "%d %s %s %s %[^\t\n]", &tdiff, domain, uuid, date,
				sha_list);

		/*!
		 * insert new UNode if it does not exist.
		 * return the corresponding UNnode in the map
		 */
		std::pair<std::map<std::string, UNode>::iterator, bool> rloc =
				Uvertices.insert(
						std::pair<std::string, UNode>(uuid,
								UNode(uuid, "", NULL)));
		UNode* rUNode = std::addressof((*(rloc.first)).second);
		//std::cout << "rUNode exist: " << !(rloc.second) << " UUID:"
		//	<< rUNode->mUuid << " mFirstarc: " << rUNode->mFirstarc
		//<< std::endl;

		/*!
		 * insert new DNode if it does not exist.
		 * return the corresponding DNode in the map
		 */
		std::pair<std::map<std::string, DNode>::iterator, bool> rdloc =
				Dvertices.insert(
						std::pair<std::string, DNode>(domain,
								DNode(domain, "", "", false, 0., 0.)));
		std::pair < std::string, DNode > rDNode = *(rdloc.first);
		//std::cout << "rDNode exist: " << !(rdloc.second) << " domain:"
		//	<< (rDNode.second).mDomain << std::endl;

		/*!
		 * create ArcNode to represent the link from UNode to DNode.
		 * break up the sha_list into an array of SHAs
		 * and add them to the ArcData section of the ArcNode
		 */
		arcnum++;
		ArcNode* new_edge = new ArcNode();
		new_edge->mKey = domain;
		(new_edge->mArcdata).mTdiff = tdiff;
		(new_edge->mArcdata).mDate = date;
		char *token = std::strtok(sha_list, " ");
		int i_sha = 0;
		while (token != NULL) {

			std::string str(token);
			(new_edge->mArcdata).mSha_list.push_back(str);
			//std::cout << "i_sha: " << i_sha << ", sha: " << token << std::endl;
			token = std::strtok(NULL, " ");
			i_sha++;
			if (i_sha > 9)
				break;
		}

		/*std::cout << "new_edge: mKey: " << new_edge->mKey << " t_diff: "
		 << (new_edge->mArcdata).mTdiff << std::endl;
		 std::cout << "----mArcdata:" << std::endl;
		 new_edge->PrintData();
		 std::cout << std::endl;
		 */

		/*!
		 * insert the ArcNode to the UNode's linked list.
		 * if the corresponding UNode exists, find the last ArcNode
		 * and append new_edge to mNextarc of the last ArcNode
		 */
		int i_node = 1;
		ArcNode* p_next = rUNode->mFirstarc;
		if (!p_next) {
			rUNode->mFirstarc = new_edge;
		} else {
			while (p_next->mNextarc) {
				p_next = p_next->mNextarc;
				i_node++;
			}
			p_next->mNextarc = new_edge;
		}
		//std::cout << "new_edge added as the " << i_node
		//		<< " ArcNode of the UNode, domain: " << domain << std::endl;
		std::cout << std::endl;

	}

	vexnum = Uvertices.size() + Dvertices.size();
	computeDNodeScore(ref_date);
	std::cout << "Number of Nodes: " << vexnum << ", Numbers of edges: "
			<< arcnum << std::endl;

	return 1;
}

int UDGraph::numberOfDays(std::string old_date, std::string new_date) {

	int t_year = std::stoi(new_date.substr(0, 4));
	int t_month = std::stoi(new_date.substr(4, 2));
	int t_day = std::stoi(new_date.substr(6, 2));

	int year = std::stoi(old_date.substr(0, 4));
	int month = std::stoi(old_date.substr(4, 2));
	int day = std::stoi(old_date.substr(6, 2));

	int days = 365 * (t_year - year) + 30 * (t_month - month) + (t_day - day);

	return days;
}

int UDGraph::computeDNodeScore(const char* today) {

	for (std::map<std::string, UNode>::iterator it = Uvertices.begin();
			it != Uvertices.end(); it++) {
		ArcNode* edge = ((*it).second).mFirstarc;
		while (edge) {
			std::string domain = edge->mKey;
			std::string date = (edge->mArcdata).mDate;
			int days = numberOfDays(date, today);
			std::cout << " #days: " << days << std::endl;
			int t_diff = (edge->mArcdata).mTdiff;
			float d_score = score(t_diff, days);
			if (Dvertices.find(domain) == Dvertices.end()) {
				std::cout << "ERROR " << domain
						<< " does not exist in Dvertices" << std::endl;
				return 0;
			}
			if (t_diff <= 0)
				Dvertices[domain].mPre_score_sum += d_score;
			else
				Dvertices[domain].mPost_score_sum += d_score;

			if (Dvertices[domain].mDate == "")
				Dvertices[domain].mDate = today;
			edge = edge->mNextarc;
		}
	}

	return 1;
}

float UDGraph::score(int t_diff, int days) {

	float sigma = 30.0;
	float T_half = 60.0; //! score drops by a factor of e^-1 in 2 months
	float dscore = exp(-pow(t_diff / sigma, 2.0));
	return dscore * exp(-days / T_half);
//	return dscore;
}

float UDGraph::damping(int days) {

	float T_half = 60.0;
	return exp(-days / T_half);
}

int UDGraph::reportScores() {

	//! sort the scores
	std::vector < std::pair<float, std::string> > post_scores;
	std::vector < std::pair<float, std::string> > pre_scores;
	for (std::map<std::string, DNode>::iterator it = Dvertices.begin();
			it != Dvertices.end(); it++) {

		//! have DNode record the date the scores are calculated.
		((*it).second).PrintData();
		std::string domain = (*it).second.mDomain;
		float post_score = (*it).second.mPost_score_sum;
		float pre_score = (*it).second.mPre_score_sum;
		if (post_score > 0)
			post_scores.push_back(
					std::pair<float, std::string>(post_score, domain));
		if (pre_score > 0)
			pre_scores.push_back(
					std::pair<float, std::string>(pre_score, domain));

	}
	std::sort(pre_scores.begin(), pre_scores.end());
	std::sort(post_scores.begin(), post_scores.end());

	//! Print the scores of each domain
	char outfilename_pre[50];
	sprintf(outfilename_pre, "pre_score.txt");
	std::fstream outfile_pre(outfilename_pre, std::ios::out);
	for (std::vector<std::pair<float, std::string> >::iterator it =
			pre_scores.begin(); it != pre_scores.end(); it++) {

		std::string domain = (*it).second;
		float pre_score = (*it).first;
		outfile_pre << domain << " " << pre_score << std::endl;
	}

	char outfilename_post[50];
	sprintf(outfilename_post, "post_score.txt");
	std::fstream outfile_post(outfilename_post, std::ios::out);
	for (std::vector<std::pair<float, std::string> >::iterator it =
			post_scores.begin(); it != post_scores.end(); it++) {

		std::string domain = (*it).second;
		float post_score = (*it).first;
		outfile_post << domain << " " << post_score << std::endl;
	}

	return 1;

}

int UDGraph::updateUDGraph(const char* new_infection_file,
		const char* ref_date) {

	//! first create a small scale UDGraph to cache the update.
	UDGraph updates;
	updates.createUDGraph(new_infection_file, ref_date);
	if (!mergeUDGraph(*this, updates, ref_date)) {
		std::cout << "UDGraph::updateUDGraph() ERROR!" << std::endl;
	}
	std::cout << "after update, number of nodes: " << vexnum
			<< ", numbers of edges: " << arcnum << std::endl;

	return 1;
}

int UDGraph::mergeUDGraph(UDGraph& main_graph, const UDGraph& new_graph,
		const char* ref_date) {

	std::map < std::string, UNode > &main_UNodes = main_graph.Uvertices;
	std::map < std::string, DNode > &main_DNodes = main_graph.Dvertices;

	const std::map<std::string, UNode> &new_UNodes = new_graph.Uvertices;
	const std::map<std::string, DNode> &new_DNodes = new_graph.Dvertices;

	for (std::map<std::string, UNode>::const_iterator it = new_UNodes.begin();
			it != new_UNodes.end(); it++) {
		/*!
		 * if UNnode in new_graph also exists in main_graph, append all its
		 * ArcNode to the end of the same UNode in main_graph.
		 * else add the entire UNode and its list of ArcNode to new_graph.
		 */
		std::pair<std::map<std::string, UNode>::iterator, bool> iloc =
				main_UNodes.insert(*it);
		/*!
		 * exist = true: the corresponding UNode exist in main_graph, iloc.first
		 * points to its location in main_UNodes.
		 * exist = false: the corresponding UNode does not exist and has been
		 * inserted in to the main_graph
		 */
		bool exist = !(iloc.second);
		if (exist) {

			ArcNode* new_edge_list = ((*it).second).mFirstarc;

			UNode* mUNode = std::addressof((*(iloc.first)).second);
			//! go to the last ArcNode of mUNode.
			ArcNode* p_next = mUNode->mFirstarc;
			if (!p_next) {
				mUNode->mFirstarc = new_edge_list;
			} else {
				while (p_next->mNextarc) {
					p_next = p_next->mNextarc;
				}
				p_next->mNextarc = new_edge_list;
			}
		}
	}

	//! combine DNodes and domain scores.
	for (std::map<std::string, DNode>::const_iterator it = new_DNodes.begin();
			it != new_DNodes.end(); it++) {
		/*!
		 * if a DNode in new_graph also exists in main_graph combine
		 * its score with the scores of the same DNode in main_graph
		 * if the corresponding DNode does not exist, insert it to
		 * the main_graph
		 */
		std::pair<std::map<std::string, DNode>::iterator, bool> dloc =
				main_DNodes.insert(*it);
		bool d_exist = !(dloc.second);
		if (d_exist) {

			std::map<std::string, DNode>::iterator dit = dloc.first;

			//! DNode exists in main_graph.
			std::string old_date = ((*dit).second).mDate; //!< date when the old scores are calculated.
			std::string update_date(ref_date);
			int days = numberOfDays(old_date, update_date);

			float old_pre_score = ((*dit).second).mPre_score_sum;
			float update_pre_score = ((*it).second).mPre_score_sum;
			((*dit).second).mPre_score_sum = old_pre_score * damping(days)
					+ update_pre_score;

			float old_post_score = ((*dit).second).mPost_score_sum;
			float update_post_score = ((*it).second).mPost_score_sum;
			((*dit).second).mPost_score_sum = old_post_score * damping(days)
					+ update_post_score;
		}
	}

	return 1;

}

int main() {

	UDGraph ud;
	ud.createUDGraph("/Users/yuxpan/fireamp/crypto_infections_1.txt",
			"20151025");
	ud.updateUDGraph("/Users/yuxpan/fireamp/crypto_infections_2.txt",
			"20151025");
	ud.reportScores();

	return 1;
}
