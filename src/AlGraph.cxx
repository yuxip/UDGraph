/*! \brief UUID-Domain Graph.
 *
 * Created on: Oct. 24, 2015
 * Author: Yuxi Pan
 *
 * implement adjacency list for graph structure
 * and depth first / breath first traversal
 */

#include<cstdlib>
#include<string>
#include<cstring>
#include<vector>
#include<map>
#include<iostream>
#include<set>
#include<cmath>
#include<ctime>
#include<fstream>
#include<algorithm>

//! UUID-domain bi-partite graph.
/*! graph edges from UUID node to domain/ip node */

//! The data element of an edge
class ArcData {

public:
	ArcData() {

		mTdiff = 999;
	}

	void Print() {
		std::cout << " Date: " << mDate << std::endl;
		std::cout << "t_diff: " << mTdiff << std::endl;
		std::cout << "---------------SHA list: " << std::endl;
		for (std::vector<std::string>::iterator it = mSha_list.begin();
				it != mSha_list.end(); it++)
			std::cout << (*it) << std::endl;

	}

	int mTdiff;
	std::vector<std::string> mSha_list;
	std::string mDate;
//	std::string mDtag;

};

//! The edge class.
class ArcNode {

public:
	ArcNode() {
		mNextarc = NULL;
	}

	std::string mKey; //!< key of the domain/ip node in the DNode map. Currently the key is domain name
	ArcNode* mNextarc;  //!< next edge (starts with the same UUID node).
	void PrintData() {
		mArcdata.Print();
	}

	//! edge data
	/*!
	 * contains detailed info on the infection.
	 * date, the set of SHAs initiated the contact,
	 * detection tags of the SHA, time difference, etc.
	 */
	ArcData mArcdata;

};

//! The UUID node.
class UNode {

public:
	UNode() {
		mUuid = "";
		mGuid = "";
		mFirstarc = NULL;

	}

	UNode(std::string uuid, std::string guid, ArcNode* firstarc) :
			mUuid(uuid), mGuid(guid), mFirstarc(firstarc) {
	}

	UNode(const UNode& other) {
		mUuid = other.mUuid;
		mGuid = other.mGuid;
		mFirstarc = other.mFirstarc;
	}

	void PrintData() {
		std::cout << "UUID: " << mUuid << "GUID: " << mGuid << std::endl;
	}

	std::string mUuid;	//!< UUID/end-point id.
	std::string mGuid;	//!< business group id.
	ArcNode* mFirstarc;  //!< first edge in linked list.
};

//! Domain(IP) node
class DNode {

public:
	DNode() {
		mDomain = "";
		mIp = "";
		mReptool_tag = false;
		mPre_score_sum = 0;
		mPost_score_sum = 0;
	}

	DNode(std::string domain, std::string ip, bool reptool_tag, float pre_score,
			float post_score) :
			mDomain(domain), mIp(ip), mReptool_tag(reptool_tag), mPre_score_sum(
					pre_score), mPost_score_sum(post_score) {
	}

	DNode(const DNode& other) {
		mDomain = other.mDomain;
		mIp = other.mIp;
		mReptool_tag = other.mReptool_tag;
		mPre_score_sum = other.mPre_score_sum;
		mPost_score_sum = other.mPost_score_sum;
	}

	void PrintData() {
		std::cout << "-----------------Domain-IP node: " << std::endl;
		std::cout << "Domain: " << mDomain << std::endl;
		std::cout << "IP: " << mIp << std::endl;
		std::cout << "Reptool_tag: " << mReptool_tag << std::endl;
		std::cout << "Pre_score_sum: " << mPre_score_sum << std::endl;
		std::cout << "Post_score_sum: " << mPost_score_sum << std::endl;

	}
	std::string mDomain;
	std::string mIp;
	bool mReptool_tag;    //!< whether it's already marked in reptool.
	float mPre_score_sum; //!< sum of pre-infection scores on all edges.
	float mPost_score_sum; //!< sum of post-infection scores on all edges.

};

class UDGraph {

public:
	UDGraph() {
		vexnum = 0;
		arcnum = 0;
	}

	std::map<std::string, UNode> Uvertices; //!< use uuid as the key
	std::map<std::string, DNode> Dvertices; //!< use domain name as the key
	int vexnum;
	int arcnum; //!< number of nodes and edges.

	//! build graph.
	/*!
	 * create the list of domains contacted from infection instances.
	 * begin with reading domain_ip_uuid*.txt files and mal_recheck_SHA*.txt file,
	 * each line of domain_ip_uuid*.txt is an infection instance on a particular
	 * uuid from a single day. confirm the infection by looking at the overlaps of the
	 * list of SHAs with those in mal_recheck_SHA*.txt (done by preprocess_udgraph.py,
	 * output: infection_file). then add the corresponding uuid node and the list
	 * of domain-ip node.
	 */
	int createUDGraph(const char* infection_file);

	//! compute the pre-/post-infection scores for each domain.
	/*!
	 * for each UNode loop over the ArcNode in its linked list,
	 * given the date and t_diff info on each ArcNode compute
	 * the pre-/post-infection scores for each DNode.
	 * @param today format "20151026"
	 * @return whether the calculate is successful
	 */
	int computeDNodeScore(const char* today);

	/*!
	 * load existing UDGraph then update it
	 */
	int updateUDGraph(const char* infection_file);

private:
	//! calculate the contribution to the score of a domain.
	/*!
	 *
	 * @param t_diff time difference w.r.t. *.tmp
	 * @param days number of days since today \sa computeDNodeScore
	 * @return
	 */
	float score(int t_diff, int days);

};

//! created UDGraph from batch-processed results.
/*!
 *
 * @param infection_file contains the domains contacted in each infection instances
 * format of infection_file:
 * time_difference, domain name, uuid, date, [list of malicious SHAs involved]
 * @return
 */
int UDGraph::createUDGraph(const char* infection_file) {

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
								DNode(domain, "", false, 0., 0.)));
		std::pair<std::string, DNode> rDNode = *(rdloc.first);
		//std::cout << "rDNode exist: " << !(rdloc.second) << " domain:"
		//	<< (rDNode.second).mDomain << std::endl;

		/*!
		 * create ArcNode to represent the link from UNode to DNode.
		 * break up the sha_list into an array of SHAs
		 * and add them to the ArcData section of the ArcNode
		 */
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
			ArcNode* p_cur = NULL;
			while (p_next) {
				p_cur = p_next;
				p_next = p_next->mNextarc;
				i_node++;
			}
			p_cur->mNextarc = new_edge;
		}
		//std::cout << "new_edge added as the " << i_node
		//		<< " ArcNode of the UNode, domain: " << domain << std::endl;
		std::cout << std::endl;

	}

	computeDNodeScore("20151025");

	return 1;
}

int UDGraph::computeDNodeScore(const char* today) {

	std::string s_today(today);
	int t_year = std::stoi(s_today.substr(0, 4));
	int t_month = std::stoi(s_today.substr(4, 2));
	int t_day = std::stoi(s_today.substr(6, 2));
	std::cout << "t_year: " << t_year << " t_month: " << t_month << " t_day: "
			<< t_day << std::endl;
	for (std::map<std::string, UNode>::iterator it = Uvertices.begin();
			it != Uvertices.end(); it++) {
		ArcNode* edge = ((*it).second).mFirstarc;
		while (edge) {
			std::string domain = edge->mKey;
			std::string date = (edge->mArcdata).mDate;
			int year = std::stoi(date.substr(0, 4));
			int month = std::stoi(date.substr(4, 2));
			int day = std::stoi(date.substr(6, 2));
			int days = 365 * (t_year - year) + 30 * (t_month - month)
					+ (t_day - day);
			std::cout << "year: " << year << " month: " << month << " day: "
					<< day << " #days: " << days << std::endl;
			int t_diff = (edge->mArcdata).mTdiff;
			float d_score = score(t_diff, days);
			if (Dvertices.find(domain) == Dvertices.end()) {
				std::cout << "ERROR " << domain
						<< " does not exist in Dvertices" << std::endl;
			}
			if (t_diff <= 0)
				Dvertices[domain].mPre_score_sum += d_score;
			else
				Dvertices[domain].mPost_score_sum += d_score;

			edge = edge->mNextarc;
		}
	}

	//! sort the scores
	std::vector < std::pair<float, std::string> > post_scores;
	std::vector < std::pair<float, std::string> > pre_scores;
	for (std::map<std::string, DNode>::iterator it = Dvertices.begin();
			it != Dvertices.end(); it++) {

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
	sprintf(outfilename_pre, "pre_score_%s.txt", today);
	std::fstream outfile_pre(outfilename_pre, std::ios::out);
	for (std::vector<std::pair<float, std::string> >::iterator it =
			pre_scores.begin(); it != pre_scores.end(); it++) {

		std::string domain = (*it).second;
		float pre_score = (*it).first;
		outfile_pre << domain << " " << pre_score << std::endl;
	}

	char outfilename_post[50];
	sprintf(outfilename_post, "post_score_%s.txt", today);
	std::fstream outfile_post(outfilename_post, std::ios::out);
	for (std::vector<std::pair<float, std::string> >::iterator it =
			post_scores.begin(); it != post_scores.end(); it++) {

		std::string domain = (*it).second;
		float post_score = (*it).first;
		outfile_post << domain << " " << post_score << std::endl;
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

int UDGraph::updateUDGraph(const char* infection_file) {

	return 1;
}

int main() {

	UDGraph* ag = new UDGraph();
	ag->createUDGraph("/Users/yuxpan/fireamp/crypto_infections.txt");

	return 1;
}
