/*! \brief UUID-Domain Graph.
 *
 * Created on: Oct. 24, 2015
 * Author: Yuxi Pan
 *
 * implement adjacency list for graph structure
 * and depth first / breath first traversal
 */

const int MAX_UNODE_NUM = 500;
const int MAX_DNODE_NUM = 1500;

#include<cstdlib>
#include<string>
#include<list>
#include<vector>
#include<iostream>
#include<set>
#include<cmath>
#include<fstream>
//!UUID-domain bi-partite graph.
/*! graph edges from UUID node to domain/ip node */

//! The edge class.
class ArcNode {

public:
	ArcNode() {
		mAdjvex = -1;
		mNextarc = NULL;
		mPre_score = 0;
		mPost_score = 0;
	}
	;
	int mAdjvex;	    //!< index of the domain/ip node in the DNode array.
	ArcNode* mNextarc;  //!< next edge (starts with the same UUID node).
	float mPre_score; //!< contribution to the pre-infection score of the domain.
	float mPost_score; //!< contribution to the post-infection score of the domain.
//	EdgeData* edata;    //!< points to the list of (score, pre-/post-flag, date) data on the heap.

};

//! The data element of a UUID node
class UData_e {

public:
	UData_e() {
		mSha = "";
		mDate = "";
		mDtag = "";
	}
	;

	void Print() {
		std::cout << "SHA: " << mSha << ", Date: " << mDate << ", Dtag: "
				<< mDtag << std::endl;
	}

	std::string mSha;
	std::string mDate;
	std::string mDtag;

};
typedef std::vector<UData_e*> UData;

//! The UUID node.
class UNode {

public:
	UNode() {
		mUuid = "";
		mGuid = "";
		mFirstarc = NULL;

	}
	;

	void PrintData() {
		std::cout << "---------------data for uuid: " << mUuid << std::endl;
		for (UData::iterator it = mData.begin(); it != mData.end(); it++)
			(*it)->Print();
		std::cout << "----------------------------- " << std::endl;
	}

	std::string mUuid;	//!< UUID/end-point id.
	std::string mGuid;	//!< business group id.
	//! The data section.
	/*! points to the details of the infection instance
	 *  from this UUID (SHA, date, detection.tag)
	 */
	UData mData;
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
	;

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
	;
	UNode Uvertices[MAX_UNODE_NUM]; //!< replace it with dynamic allocation.
	DNode Dvertices[MAX_DNODE_NUM];
	int vexnum;
	int arcnum; //!< number of nodes and edges.

	//! build graph.
	/*!
	 * create UDGraph by reading domain_ip_uuid*.txt files and mal_recheck_SHA*.txt file,
	 * each line of domain_ip_uuid*.txt is an infection instance on a particular uuid from a single day
	 * confirm the infection by looking at the overlaps of the list of SHAs with those in
	 * mal_recheck_SHA*.txt (done by find_infection.py, output format 'uuid sha' tuple in dsfilename),
	 * then add the corresponding uuid node and the list of domain-ip node
	 */
	int createUDGraph(std::string dsfilelist);

	/*!
	 * load existing UDGraph then update it with the domain_ip_uuid*.txt file
	 */
	int updateUDGraph(std::string udsfilelist);

private:

};

int UDGraph::createUDGraph(std::string dsfilelist) {

	std::ifstream inlist(dsfilelist, std::ios::in);
	std::cout << "opening file list: " << dsfilelist << std::endl;
	if (!inlist) {
		std::cout << "ERROR opening file " << inlist << std::endl;
		return -1;
	}
	char uuid[100];
	char sha[100];
	char date[20];
	char dfilename[200];
	while (inlist >> uuid >> sha >> date) {

		sprintf(dfilename,
				"/Users/yuxpan/fireamp/domain_ip_uuid%s_sha%s_%s.txt", uuid,
				sha, date);
		std::cout << "reading " << dfilename << std::endl;
		std::ifstream infile(dfilename, std::ios::in);
		if (!infile) {
			std::cout << "ERROR opening file " << dfilename << std::endl;
			return -1;
		}
		int tdiff;
		char ip[50];
		char url[1000];
		for (std::string line; std::getline(infile, line);) {
			std::cout << line << std::endl;
			sscanf(line.c_str(), "%d,%s,%s", &tdiff, ip, url);
			std::cout << tdiff << " " << ip << " " << url << std::endl;

		}

	}

	return 1;
}

int UDGraph::updateUDGraph(std::string udsfilelist) {

	return 1;
}

int main() {

	UDGraph* ag = new UDGraph();
	ag->createUDGraph("/Users/yuxpan/fireamp/dstuple_20151006to1021.txt");

	return 1;
}
