/*
* This simulation tests the modification of the rate using prc-monitor and prc-pep.
* A client is connected to 1 AP and tranmist data with a CBR of 54Mb/s
*/

#include "ns3/dce-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
//#include "prc-monitorExample.h" // agregado para el rate
#include <sstream>
#include <fstream>


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("LamimExperiment");

// packet size generated at the AP
static const uint32_t packetSize = 1420;

void
TxVectorCallback (std::string path, WifiTxVector vector)
{
  //NS_LOG_UNCOND ((Simulator::Now ()).GetSeconds () << " " <<  vector.GetMode().GetDataRate()/1000000  << " " << (int)vector.GetTxPowerLevel());
}

class ThroughputCounter
{
public:
  ThroughputCounter();

  void RxCallback (std::string path, Ptr<const Packet> packet, const Address &from);
  void CheckThroughput ();
  Gnuplot2dDataset GetDatafile();

  uint32_t bytesTotal;
  Gnuplot2dDataset m_output;
};

ThroughputCounter::ThroughputCounter() : m_output ("Throughput Mbit/s")
{
  bytesTotal = 0;
  m_output.SetStyle (Gnuplot2dDataset::LINES);
}

void
ThroughputCounter::RxCallback (std::string path, Ptr<const Packet> packet, const Address &from)
{
  bytesTotal += packet->GetSize();
}

void
ThroughputCounter::CheckThroughput()
{
  double mbs = ((bytesTotal * 8.0) /100000);
  bytesTotal = 0;
  m_output.Add ((Simulator::Now ()).GetSeconds (), mbs);
  Simulator::Schedule (Seconds (0.1), &ThroughputCounter::CheckThroughput, this);
}

Gnuplot2dDataset
ThroughputCounter::GetDatafile()
{ return m_output; }


int main (int argc, char *argv[])
{
	double maxPower = 17;
	double minPower = 0;
	uint32_t powerLevels = 18;
	std::string manager = "ns3::ParfWifiManager";
	uint32_t rtsThreshold = 2346;
	uint32_t simuTime = 100;
	int ap0_x = 0;
	int ap0_y = 0;
	int sta0_x = 60;
	int sta0_y = 0;
	int sta1_x = 80;
	int sta1_y = 0;
	int ap1_x = 120;
	int ap1_y = 0;
	int sta2_x = 70;
	int sta2_y = 0;

   CommandLine cmd;
   cmd.Parse (argc, argv);
  
  LogComponentEnable("PrcMonitordce2", LOG_LEVEL_INFO);
  LogComponentEnable("PrcPep2", LOG_LEVEL_INFO);
  LogComponentEnable("PrcMonitordce2", LOG_PREFIX_NODE);
  LogComponentEnable("PrcPep2", LOG_PREFIX_NODE);



    // Define the APs
    NodeContainer wifiApNodes;
    wifiApNodes.Create (2);

    //Define the STAs
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (3);

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetStandard (WIFI_PHY_STANDARD_80211g); //o WIFI_PHY_STANDARD_80211g
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    //Create a channel helper in a default working state. By default, we create a channel model with a propagation delay equal to a constant, the speed of light,
    // and a propagation loss based on a log distance model with a reference loss of 46.6777 dB at reference distance of 1m
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    NetDeviceContainer wifiApDevices;
    NetDeviceContainer wifiStaDevices;
    NetDeviceContainer wifiDevices;

    // ?
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  //Connect all APs
  NetDeviceContainer csmaApDevices;
  csmaApDevices = csma.Install (wifiApNodes);
  // ?

  //Configure the STA nodes
  // original dce-lamim-1
    //wifi.SetRemoteStationManager ("ns3::AarfWifiManager", "RtsCtsThreshold", UintegerValue (rtsThreshold));
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue ("ErpOfdmRate24Mbps"));
    wifiPhy.Set ("TxPowerStart", DoubleValue (maxPower));
    wifiPhy.Set ("TxPowerEnd", DoubleValue (maxPower));

    Ssid ssid = Ssid ("AP");
    wifiMac.SetType ("ns3::StaWifiMac",
                     "Ssid", SsidValue (ssid),
                     "ActiveProbing", BooleanValue (false));
                    // "MaxMissedBeacons", UintegerValue (1));
    wifiStaDevices.Add (wifi.Install (wifiPhy, wifiMac, wifiStaNodes.Get (0)));

    ssid = Ssid ("AP");
    wifiMac.SetType ("ns3::StaWifiMac",
                     "Ssid", SsidValue (ssid),
                     "ActiveProbing", BooleanValue (false));
                   //  "MaxMissedBeacons", UintegerValue (1),
    wifiStaDevices.Add (wifi.Install (wifiPhy, wifiMac, wifiStaNodes.Get (1)));

    ssid = Ssid ("AP");
    wifiMac.SetType ("ns3::StaWifiMac",
                     "Ssid", SsidValue (ssid),
                     "ActiveProbing", BooleanValue (false));
                     // "MaxMissedBeacons", UintegerValue (1));
    wifiStaDevices.Add (wifi.Install (wifiPhy, wifiMac, wifiStaNodes.Get (2)));

    //Configure the AP nodes  configurar igual q estaciones
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",StringValue ("ErpOfdmRate24Mbps"));
    wifiPhy.Set ("TxPowerStart", DoubleValue (maxPower));
    wifiPhy.Set ("TxPowerEnd", DoubleValue (maxPower));

    ssid = Ssid ("AP"); //AP0
    wifiMac.SetType ("ns3::ApWifiMac",
                     "Ssid", SsidValue (ssid));
    wifiApDevices.Add (wifi.Install (wifiPhy, wifiMac, wifiApNodes.Get (0)));

    ssid = Ssid ("AP");//AP1
    wifiMac.SetType ("ns3::ApWifiMac",
                     "Ssid", SsidValue (ssid));
                     //"BeaconInterval", TimeValue (MicroSeconds (103424))); //for avoiding collisions);
    wifiApDevices.Add (wifi.Install (wifiPhy, wifiMac, wifiApNodes.Get (1)));

    wifiDevices.Add (wifiStaDevices);
    wifiDevices.Add (wifiApDevices);

    // mobility.
     MobilityHelper mobility;
     Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
     positionAlloc->Add (Vector (ap0_x, ap0_y, 0.0));
     positionAlloc->Add (Vector (sta0_x, sta0_y, 0.0));
     positionAlloc->Add (Vector (sta1_x, sta1_y, 0.0));
     positionAlloc->Add (Vector (ap1_x, ap1_y, 0.0));
     positionAlloc->Add (Vector (sta2_x, sta2_y, 0.0));

     mobility.SetPositionAllocator (positionAlloc);
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
     mobility.Install (wifiApNodes.Get(0));
     mobility.Install (wifiStaNodes.Get(0));
     mobility.Install (wifiStaNodes.Get(1));
     mobility.Install (wifiApNodes.Get(1));
     mobility.Install (wifiStaNodes.Get(2));

    //Configure de IP stack
    InternetStackHelper stack;
    stack.Install (wifiApNodes);
    stack.Install (wifiStaNodes);
    Ipv4AddressHelper address;

    address.SetBase ("10.1.0.0", "255.255.255.0");
    address.Assign (csmaApDevices);

    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = address.Assign (wifiDevices);
    Ipv4Address sinkAddress = i.GetAddress (0);
    Ipv4Address sinkAddress1 = i.GetAddress (1);
    Ipv4Address sinkAddress2 = i.GetAddress (2);
    uint16_t port = 9;


    //Configure the CBR generator
    PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress, port));
    ApplicationContainer apps_sink = sink.Install (wifiStaNodes.Get (0));

    OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress, port));
    onoff.SetConstantRate (DataRate ("54Mb/s"), packetSize);
    onoff.SetAttribute ("StartTime", TimeValue (Seconds (0.0))); //puede ser 10.0
    onoff.SetAttribute ("StopTime", TimeValue (Seconds (100.0)));
    ApplicationContainer apps_source = onoff.Install (wifiApNodes.Get (0));

    PacketSinkHelper sink1 ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress1, port));
    apps_sink.Add (sink1.Install (wifiStaNodes.Get (1)));

    OnOffHelper onoff1 ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress1, port));
    onoff1.SetConstantRate (DataRate ("54Mb/s"), packetSize);
    onoff1.SetAttribute ("StartTime", TimeValue (Seconds (0.0))); // puede ser 10.0
    onoff1.SetAttribute ("StopTime", TimeValue (Seconds (100.0)));
    apps_source.Add (onoff1.Install (wifiApNodes.Get (0)));

    PacketSinkHelper sink2 ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress2, port));
    apps_sink.Add (sink2.Install (wifiStaNodes.Get (2)));

    OnOffHelper onoff2 ("ns3::UdpSocketFactory", InetSocketAddress (sinkAddress2, port));
    onoff2.SetConstantRate (DataRate ("54Mb/s"), packetSize);
    onoff2.SetAttribute ("StartTime", TimeValue (Seconds (0.0))); // puede ser 10.0
    onoff2.SetAttribute ("StopTime", TimeValue (Seconds (100.0)));
    apps_source.Add (onoff2.Install (wifiApNodes.Get (1)));

    apps_sink.Start (Seconds (0.5)); //puede ser 2.0
    apps_sink.Stop (Seconds (simuTime));


  // Create one PrcMonitor application

    Ptr<PrcMonitordce2> monitor = CreateObject<PrcMonitordce2> ();
    wifiApNodes.Get (1)->AddApplication (monitor);
    monitor->SetStartTime (Seconds (10.0));
    monitor->SetStopTime (Seconds (100.0));

    Ptr<PrcMonitordce2> monitor0 = CreateObject<PrcMonitordce2> ();
    wifiApNodes.Get (0)->AddApplication (monitor0);
    monitor0->SetStartTime (Seconds (10.0));
    monitor0->SetStopTime (Seconds (100.0));

    Ptr<PrcPep2> pep = CreateObject<PrcPep2> ();
    wifiApNodes.Get (0)->AddApplication (pep);
    pep->SetStartTime (Seconds (10.0));
    pep->SetStopTime (Seconds (100.0));

    Ptr<PrcPep2> pep1 = CreateObject<PrcPep2> ();
    wifiApNodes.Get (1)->AddApplication (pep1);
    pep1->SetStartTime (Seconds (10.0));
    pep1->SetStopTime (Seconds (100.0));

    DceManagerHelper dceManager;

    dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                        StringValue ("UcontextFiberManager"));
    dceManager.Install (wifiApNodes);

    DceApplicationHelper dce;
    ApplicationContainer apps;

    dce.SetStackSize (1 << 20);

    dce.SetBinary ("./lua");
    dce.ResetArguments ();
    dce.AddArgument ("rnr/rnr.lua");
    dce.AddArgument ("rnr/config_lamim_NS3.txt"); // define los puertos rubela/ns3dcegit/files-0/rnr
    apps = dce.Install (wifiApNodes);
    apps.Start (Seconds (4.0));

    dce.SetBinary ("./lua");
    dce.ResetArguments ();
    dce.AddArgument ("lupa/lupa.lua");
    dce.AddArgument ("lupa/config_lamim_NS3.txt"); // define las AP y nombres   rubelala/ns3dcegit/files-0/lupa
    apps = dce.Install (wifiApNodes);
    apps.Start (Seconds (5.0));

    dce.SetBinary ("./lua");
    dce.ResetArguments ();
    dce.AddArgument ("fsm/setfsm_rate_loss1.lua");
    apps = dce.Install (wifiApNodes);
    apps.Start (Seconds (10.0));

  //------------------------------------------------------------
    //-- Setup stats and data collection
    //--------------------------------------------

  ThroughputCounter* throughputCounter = new ThroughputCounter();

  Config::Connect ("/NodeList/1/ApplicationList/*/$ns3::PacketSink/Rx",
    				MakeCallback (&ThroughputCounter::RxCallback, throughputCounter));

  throughputCounter->CheckThroughput();

  Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/RemoteStationManager/$ns3::RuleBasedWifiManager/DoGetDataTxVector",
                                MakeCallback (&TxVectorCallback));


  // Calculate Throughput using Flowmonitor
  //
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> fMonitor = flowmon.InstallAll();
//
  Simulator::Stop (Seconds (40.0)); //era 100
  Simulator::Run ();


    //monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = fMonitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
  	Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if ((t.sourceAddress=="10.1.1.4" && t.destinationAddress == "10.1.1.1")) //era 10.1.1.2
      {
    	NS_LOG_UNCOND("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n");
        NS_LOG_UNCOND("  Tx Bytes:   " << i->second.txBytes << "\n");
        NS_LOG_UNCOND("  Rx Bytes:   " << i->second.rxBytes << "\n");
        NS_LOG_UNCOND("  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n");

      }
    }

  std::ofstream outfile ("throughput.plt");
  Gnuplot gnuplot = Gnuplot("th.eps", "Throughput");
  gnuplot.SetTerminal("post eps enhanced");
  gnuplot.AddDataset (throughputCounter->GetDatafile());
  gnuplot.GenerateOutput (outfile);

  Simulator::Destroy ();

  return 0;
}
