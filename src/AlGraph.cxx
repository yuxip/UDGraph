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
#include<fstream>
//!UUID-domain bi-partite graph.
/*! graph edges from UUID node to domain/ip node */

//! The data element of an edge
class ArcData {

public:
	ArcData() {
//		mDate = "";
//		mDtag = "";
	}

	void Print() {
		std::cout << " Date: " << mDate << std::endl;
		std::cout << "---------------SHA list: " << std::endl;
		for (std::vector<std::string>::iterator it = mSha_list.begin();
				it != mSha_list.end(); it++)
			std::cout << (*it) << std::endl;

	}

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

	/*!
	 * load existing UDGraph then update it
	 */
	int updateUDGraph(const char* infection_file);

private:

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
		std::cout << "rUNode exist: " << !(rloc.second) << " UUID:"
				<< rUNode->mUuid << " mFirstarc: " << rUNode->mFirstarc
				<< std::endl;

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
		//std::cout << "new_edge: mKey: " << new_edge->mKey << std::endl;
		//std::cout << "----mArcdata:" << std::endl;
		//new_edge->PrintData();
		//std::cout << std::endl;

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
		std::cout << "new_edge added as the " << i_node
				<< " ArcNode of the UNode, domain: " << domain << std::endl;
		std::cout << std::endl;

	}

	return 1;
}

int UDGraph::updateUDGraph(const char* infection_file) {

	return 1;
}

int main() {

	UDGraph* ag = new UDGraph();
	ag->createUDGraph("/Users/yuxpan/fireamp/crypto_infections.txt");

	return 1;
}
