/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

#include <string>

using namespace ns3;



NS_LOG_COMPONENT_DEFINE ("part 3");

int FlowOutput(Ptr<FlowMonitor> flowmon, FlowMonitorHelper flowmonHelper)
{
        flowmon->CheckForLostPackets (); //check all packets have been sent or completely lost
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
        std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats (); // pull stats from flow monitor

        for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
        {//iterate through collected flow stats and find the traffic from and to Access point.
                Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
                
                // We are only interested in the traffic that the AP receives or transmits hence the or statement.
                if (t.sourceAddress == Ipv4Address("10.1.1.1") || t.destinationAddress == Ipv4Address("10.1.1.1")) 
                //use the node addresses to select the flow (or flows) you want
                {
                        NS_LOG_UNCOND("Flow : " << iter->first << " Src : " << t.sourceAddress << " Dst : " << t.destinationAddress);
                        NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
                        NS_LOG_UNCOND("Aggregate Throughput: " << iter->second.rxBytes * 8.0 /(1024)/ (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())   << " Kbps");
                }

        }
        return 0;
}

int 
main (int argc, char *argv[])
{
        bool verbose = false;
        
        // Set to true for this question.
        bool rayleigh = true;
  
        CommandLine cmd;
        cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose); 
        //parameter name, description, variable that will take the value read from the command line

        cmd.Parse (argc,argv);
  
        if (verbose)
        {
                LogComponentEnable ("PacketSink", LOG_LEVEL_INFO); //packet sink should write all actions to the command line
        }

        //seed number should change between runs if running multiple simulations.
        SeedManager::SetSeed (23);

        //---------------------------------------------------------------------------------
        //--------------------------- Node Creation ---------------------------------------
        //---------------------------------------------------------------------------------

        NodeContainer wifiStaNodes;
        wifiStaNodes.Create (50);
        NodeContainer wifiApNode;
        wifiApNode.Create(1);

         
        //---------------------------------------------------------------------------------
        //-------------------------- Channel Creation -------------------------------------
        //---------------------------------------------------------------------------------
        
        YansWifiChannelHelper channel; //create helpers for the channel and phy layer and set propagation configuration here.
        channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
        channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");
          
        // Fading scenario with rayleigh fading implemented if raleigh is set to true.      
        if(rayleigh){
                channel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
	                                   "m0", DoubleValue(1.0), "m1", DoubleValue(1.0), "m2", DoubleValue(1.0)); 
                //combining log and nakagami to have both distance and rayleigh fading (nakami with m0, m1 and m2 =1 is rayleigh)
        }
        YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
        phy.SetChannel (channel.Create ());

        //---------------------------------------------------------------------------------
        //--------------------------- WIFI Settings ---------------------------------------
        //---------------------------------------------------------------------------------        
        
        //create helper for the overall wifi setup and configure a station manager
        WifiHelper wifi = WifiHelper::Default (); 
        wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211g);
        
        //wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
        wifi.SetRemoteStationManager ("ns3::CaraWifiManager");
        
        //create a mac helper and configure for station and AP and install
        NqosWifiMacHelper mac = NqosWifiMacHelper::Default (); 

        Ssid ssid = Ssid ("example-ssid");
        mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (true));

        NetDeviceContainer staDevices;
        staDevices = wifi.Install (phy, mac, wifiStaNodes);

        mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

        NetDeviceContainer apDevices;
        apDevices = wifi.Install (phy, mac, wifiApNode);

        //---------------------------------------------------------------------------------
        //----------------------- Mobility and Positioning --------------------------------
        //---------------------------------------------------------------------------------

        MobilityHelper mobility;

        // The AP is located at coordinates 0,0.
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        mobility.Install (wifiApNode);

        // The station nodes are allocated randomly within a circle with a radius of 25m. 
        mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
                                        "rho", DoubleValue(25.0),
                                        "X", DoubleValue (0.0),
                                        "Y", DoubleValue (0.0));


        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");       
        mobility.Install (wifiStaNodes);
        
        //install the internet stack on both nodes
        InternetStackHelper stack; 
        stack.Install (wifiApNode);
        stack.Install (wifiStaNodes);

        //---------------------------------------------------------------------------------
        //--------------------------- IP Assignments --------------------------------------
        //---------------------------------------------------------------------------------
        Ipv4AddressHelper address; //assign IP addresses to all nodes
        address.SetBase ("10.1.1.0", "255.255.255.0");
        address.Assign (staDevices);
        Ipv4InterfaceContainer apAddress = address.Assign (apDevices); //we need to keep AP address accessible as we need it later
        

        //---------------------------------------------------------------------------------
        //--------------------- Application Configuration ---------------------------------
        //---------------------------------------------------------------------------------
        OnOffHelper onoff ("ns3::UdpSocketFactory", Address ()); //create a new on-off application to send data
        std::string dataRate = "20Mib/s"; //data rate set as a string, see documentation for accepted units
        onoff.SetConstantRate(dataRate, (uint32_t)1024); //set the onoff client application to CBR mode

        AddressValue remoteAddress (InetSocketAddress (apAddress.GetAddress (0), 8000));
        //specify address and port of the AP as the destination for on-off application's packets
        onoff.SetAttribute ("Remote", remoteAddress);
        ApplicationContainer apps = onoff.Install (wifiStaNodes);//install onoff application on stanode and configure start/stop times
        UniformVariable var;
        

        //---------------------------------------------------------------------------------
        //---------------------------- Simulation -----------------------------------------
        //---------------------------------------------------------------------------------
        apps.Start(Seconds(var.GetValue(0, 0.1)));
        apps.Stop (Seconds (10));

        PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress (apAddress.GetAddress (0), 8000)); 
        //create packet sink on AP node with address and port that on-off app is sending to
        apps.Add(sink.Install(wifiApNode.Get(0)));
        
        Simulator::Stop (Seconds (10.0)); //define stop time of simulator

        Ptr<FlowMonitor> flowmon; //create an install a flow monitor to monitor all transmissions around the network
        FlowMonitorHelper flowmonHelper;
        flowmon = flowmonHelper.InstallAll();

        Simulator::Run (); //run the simulation and destroy it once done
        Simulator::Destroy ();
        FlowOutput(flowmon, flowmonHelper);
  
        return 0;
}
