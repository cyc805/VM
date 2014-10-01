#ifndef NS3_MYUTIL_H
#define NS3_MYUTIL_H

#include <stdio.h>
#include <fstream>
#include <iostream>
#include "ns3/pcap-file-wrapper.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/trace-helper.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/flow-classifier.h"

using namespace ns3;

namespace czhe {

enum Event {
	TCP_FlowStart, TCP_FlowComplete, TCP_Sender_RTO
};

/*Convert integer to string*/
std::string i2s(uint i);

std::string d2s(double d);

/*
 Create a ascii/plain file, return the pointer.

 Usage:
 Ptr<OutputStreamWrapper> f = NewAsciiFile("test.txt")
 f->GetStream() << "line1" <<std::endl;
 f->GetStream() << "line2" <<std::endl;

 */
Ptr<OutputStreamWrapper> OpenAsciiFile(std::string fileName,
		std::ios::openmode openmode = std::ios::out);

/*
 Create a pcap file, return the pointer.

 Usage:
 Ptr<PcapFileWrapper> file = NewPcapFile("test.pcap", PcapHelper::DLT_PPP)
 Ptr<const Packet> p;
 file->Write (Simulator::Now (), p);

 */
Ptr<PcapFileWrapper> OpenPcapFile(std::string fileName, uint32_t dataLinkType,
		std::ios::openmode openmode = std::ios::out);

/*
 Callback function for TraceSource value.

 Usage (trace congestion window size):
 ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&ValueChange, NewAsciiFile("sixth.cwnd")));

 */
void ValueChange(Ptr<OutputStreamWrapper> stream, uint32_t oldValue,
		uint32_t newValue);
/*
 Callback function for packet tracing.

 Usage:
 devices.Get(1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&PktRecord, NewPcapFile("sixth.pcap")));

 */
void PktRecord(Ptr<PcapFileWrapper> file, Ptr<const Packet> p);

void EventRecord(Ptr<OutputStreamWrapper> stream, double time,
		std::string event, std::string flowId, std::string value = "");

std::string GetFiveTuple(Ipv4Address src_ip, uint src_port, Ipv4Address dst_ip,
		uint dst_port, uint protocol = ns3::TcpL4Protocol::PROT_NUMBER);

std::string GetFiveTuple(Ipv4EndPoint* endPoint, bool isRcver = false);

std::string GetFiveTuple(Address src, Address dst);

std::string GetFiveTuple(Ptr<Packet> pkt);

std::pair<uint, double> tenant_hash(Ptr<Packet> pkt);

} // namespace czhe

namespace ns3 {

class MyOutputStreamWrapper: public Object {
public:
	static TypeId GetTypeId(void);
	MyOutputStreamWrapper();
	MyOutputStreamWrapper(Ptr<OutputStreamWrapper> osw);
	virtual ~MyOutputStreamWrapper();

public:
	Ptr<OutputStreamWrapper> m_osw;
};

} // namespace ns3

#endif /* NS3_MYUTIL_H */
