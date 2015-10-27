/*
 * UDGraph.h
 *
 *  Created on: Oct 27, 2015
 *      Author: yuxpan
 */

#ifndef UDGRAPH_H_
#define UDGRAPH_H_

#include<vector>
#include<map>
#include<iostream>
#include<string>

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

#endif /* UDGRAPH_H_ */
