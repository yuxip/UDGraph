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
	void Print();

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

	UNode(std::string uuid, std::string guid, ArcNode* firstarc);
	UNode(const UNode& other);
	void PrintData();

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
		mDate = "";
		mReptool_tag = false;
		mPre_score_sum = 0;
		mPost_score_sum = 0;
	}

	DNode(std::string domain, std::string ip, std::string date,
			bool reptool_tag, float pre_score, float post_score);
	DNode(const DNode& other);
	void PrintData();

	std::string mDomain;
	std::string mIp;
	std::string mDate; //!< date the scores are calculated, this will speed up the update
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
	 * @param infection_file
	 * @param ref_date reference date of the domain score (e.g. today)
	 * @return status code
	 *
	 * create the list of domains contacted from infection instances.
	 * begin with reading domain_ip_uuid*.txt files and mal_recheck_SHA*.txt
	 * file, each line of domain_ip_uuid*.txt is an infection instance on a
	 * particular uuid from a single day. confirm the infection by looking at
	 * the overlaps of the list of SHAs with those in mal_recheck_SHA*.txt
	 * (done by preprocess_udgraph.py, output: infection_file). then add the
	 * corresponding uuid node and the list of domain-ip node.
	 *
	 */
	int createUDGraph(const char* infection_file, const char* ref_date);

	//! load existing UDGraph then update it.
	/*!
	 * @param new_infection_file
	 * @param ref_date
	 * @return status code
	 *
	 * create a smaller scale UDGraph to cache the update,
	 * then merge with the main UDGraph.
	 * e.g.
	 * if UNode exists in the main UDGraph: append the ArcNodes
	 * if DNode exists in the main UDGraph:
	 * 		combine the scores: old_score*f_damp(day_diff) + new_score
	 *
	 */
	int updateUDGraph(const char* new_infection_file, const char* ref_date);

	//! dump all of the domain scores
	int reportScores();

private:

	//! compute the pre-/post-infection scores for each domain.
	/*!
	 * @param today reference date, format "20151026"
	 * @return status code
	 *
	 * for each UNode loop over the ArcNode in its linked list,
	 * given the date and t_diff info on each ArcNode compute
	 * the pre-/post-infection scores for each DNode.
	 */
	int computeDNodeScore(const char* today);

	//! calculate the contribution to the score of a domain.
	/*!
	 * @param t_diff time difference w.r.t. *.tmp
	 * @param days number of days since today \sa computeDNodeScore
	 * @return
	 */
	float score(int t_diff, int days);

	//! merge new_graph into main_graph.
	/*!
	 * @param main_graph The main UDGraph
	 * @param new_graph The update portion of the UDGraph
	 * @param ref_date The reference date \sa updateUDGraph
	 * @return status code
	 *
	 * merge UNodes, DNodes, append new ArcNode
	 * and combine domain scores.
	 */
	int mergeUDGraph(UDGraph& main_graph, const UDGraph& new_graph,
			const char* ref_date);

	/*!
	 * @param old_date
	 * @param new_date
	 * @return the number of days between old_date and new_date
	 */
	int numberOfDays(std::string old_date, std::string new_date);
	float damping(int days); //!< damping factors for propagating scores 'days' to the future;
};

#endif /* UDGRAPH_H_ */
