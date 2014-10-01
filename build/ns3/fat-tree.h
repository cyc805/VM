#ifndef FAT_TREE_H
#define FAT_TREE_H

#include <stdio.h>
#include <iostream>
#include <map>
#include <algorithm>
#include "ns3/object.h"
#include "ns3/event-id.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/data-rate.h"
#include <set>

using namespace ns3;
namespace ns3 {

class BWEstimator: public Object {
public:
	static TypeId GetTypeId(void);
	BWEstimator();
	virtual ~BWEstimator();

	void Trace(uint nByte);
	double GetBW();

private:
	uint traced_bytes;
	double last_estimate_time;
};

/**
 * (r,b) token bucket regulator.
 */
class TBRegulator: public Object {
public:
	static TypeId GetTypeId(void);
	TBRegulator();
	virtual ~TBRegulator();
	void AddToken();
	void ReduceToken(uint bytes);
	double GetToken();
	uint GetB();
	DataRate GetMinR();
	DataRate GetMaxR();
	DataRate GetR();

private:
	DataRate min_r_bps; // default/min token fill rate (in bps)
	DataRate max_r_bps; // max token fill rate (in bps)
	DataRate r_bps; // current token fill rate (in bps)
	uint b_byte; // default bucket size/burst size (in bytes)
	double token_byte; // current tokens/bucket size (in bytes)
	Time last_update_time;
};

/**
 * Balance/reassign fill rates among token bucket regulators.
 */
class RateBalancer: public Object {
public:
	static TypeId GetTypeId(void);
	RateBalancer();
	virtual ~RateBalancer();
	void AddTBR(Ptr<TBRegulator> tbr);
	uint GetTotalR();
	uint GetTotalMinR();
	void Refresh();
	void Share(Ptr<TBRegulator> from, std::vector<Ptr<TBRegulator> > to);
	void Getback(Ptr<TBRegulator> to, std::vector<Ptr<TBRegulator> > from);

	void ViolationCheck();

	std::pair<std::vector<Ptr<TBRegulator> >, std::vector<Ptr<TBRegulator> > > PartByMinR(
			std::vector<Ptr<TBRegulator> > dynamic_tbrs);
	std::pair<std::vector<Ptr<TBRegulator> >, std::vector<Ptr<TBRegulator> > > PartByFullToken(
			std::vector<Ptr<TBRegulator> > dynamic_tbrs);
private:
	Time interval; // refresh interval;
	DataRate link_bps; // total link rate
	EventId refreshEvent;
//	bool enable_dynamic_rate;

	std::vector<Ptr<TBRegulator> > m_tbrs;
	std::vector<Ptr<TBRegulator> > dynamic_tbrs;
};

/**
 * (r,b) Token Bucket Queue
 */
class TBQueue: public Queue {
public:
	static TypeId GetTypeId(void);
	TBQueue();
	virtual ~TBQueue();

	Ptr<TBRegulator> GetTBR(void);
	Time NextSrvTime();

	uint GetNBytes();

private:
	virtual bool DoEnqueue(Ptr<Packet> p);
	virtual Ptr<Packet> DoDequeue(void);
	virtual Ptr<const Packet> DoPeek(void) const;

	Ptr<Packet> RealDequeue();
	Ptr<TBRegulator> tbr;
	std::deque<Ptr<Packet> > m_pkts; // back queue

	uint m_nBytes; // current queue length
	uint m_maxBytes; // max queue length
};

/**
 * Deficit Round Robin Queue.
 * Implemented using round robin on token bucket queues.
 */
class DRR: public Queue {
public:
	static TypeId GetTypeId(void);
	DRR();
	virtual ~DRR();

	uint GetLongestFlowId(void);
	uint GetTotalBytes(void) const;
	uint GetNBytes();
	std::deque<uint> getNonemptyIds();
	Time NextSrvTime();
	void SetHashCallback(Callback<std::pair<uint, double>, Ptr<Packet> > hash);

private:
	virtual bool DoEnqueue(Ptr<Packet> p);
	virtual Ptr<Packet> DoDequeue(void);
	virtual Ptr<const Packet> DoPeek(void) const;

//	void DropLongest(uint bytes);

	Callback<std::pair<uint, double>, Ptr<Packet> > m_hash;
	Ptr<TBQueue> InitQueue(double weight);

	uint32_t m_maxBytes; // max queue length for EACH round robin queue.
	std::map<uint, Ptr<TBQueue> > flowid_queue_map;
	std::vector<Ptr<TBQueue> > rrqs; // round robin queues
	uint rr_pointer; // round robin pointer
//	std::map<uint, uint> flowid_quantum_map;
//	std::map<uint, double> flowid_weight_map;
//	uint m_quantum; // in bytes

	DataRate link_bps; // output link rate
	Ptr<RateBalancer> rate_balancer;

};

/**
 * Use token bucket to limit receiving rate.
 */
class RcvLimiter: public Object {
public:
	static TypeId GetTypeId(void);
	RcvLimiter();
	virtual ~RcvLimiter();

	Ptr<Packet> RateFilter(Ptr<Packet> pkt);
	void Refresh();

private:
	std::map<std::pair<uint, std::string>, Ptr<TBRegulator> > id_tbr_map;
	std::map<uint, Ptr<TBRegulator> > flowid_tbr_map;
	DataRate link_bps; // link rate
	Callback<std::pair<uint, double>, Ptr<Packet> > m_hash;

	Ptr<RateBalancer> rate_balancer;
	std::set<std::string> flows_congestion_notified;

	Time interval;
};

} // namespace ns3
#endif /* FAT_TREE_H */
