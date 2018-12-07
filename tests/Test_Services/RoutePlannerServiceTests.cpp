#include "gtest/gtest.h"

#include "ServiceTesting.h"
#include "TestLmcpObjectNetworkClient.h"

#include "RoutePlannerService.h"

#include "uxas/messages/route/RoutePlanRequest.h"
#include "uxas/messages/route/RoutePlanResponse.h"

TEST(RoutePlannerServiceTest, ResponseIdMatchesRequestId)
{
    auto client = std::make_shared<test::TestLmcpObjectNetworkClient>(400, "Aircraft", 10);
    uxas::service::RoutePlannerService service(client);

    auto avs = test::utility::CreateMessage(
        "<AirVehicleState Series=\"CMASI\">"
            "<Airspeed>18.275354385376</Airspeed>"
            "<VerticalSpeed>0</VerticalSpeed>"
            "<WindSpeed>0</WindSpeed>"
            "<WindDirection>0</WindDirection>"
            "<ID>400</ID>"
            "<u>18.275354385376</u>"
            "<v>0</v>"
            "<w>0</w>"
            "<udot>0</udot>"
            "<vdot>0</vdot>"
            "<wdot>0</wdot>"
            "<Heading>90.1087112426758</Heading>"
            "<Pitch>0</Pitch>"
            "<Roll>2</Roll>"
            "<p>-1.04719758033752</p>"
            "<q>-0.00132825796026736</q>"
            "<r>-0.0380363315343857</r>"
            "<Course>90.1087112426758</Course>"
            "<Groundspeed>18.1427421569824</Groundspeed>"
            "<Location>"
                "<Location3D Series=\"CMASI\">"
                    "<Latitude>45.3170999827963</Latitude>"
                    "<Longitude>-120.99228463463</Longitude>"
                    "<Altitude>700</Altitude>"
                    "<AltitudeType>MSL</AltitudeType>"
                "</Location3D>"
            "</Location>"
            "<EnergyAvailable>99.9999694824219</EnergyAvailable>"
            "<ActualEnergyRate>0.000277999992249534</ActualEnergyRate>"
            "<PayloadStateList/>"
            "<CurrentWaypoint>0</CurrentWaypoint>"
            "<CurrentCommand>0</CurrentCommand>"
            "<Mode>FlightDirector</Mode>"
            "<AssociatedTasks/>"
            "<Time>1510</Time>"
            "<Info/>"
        "</AirVehicleState>");
    ASSERT_NE(avs, nullptr);

    ASSERT_TRUE(test::service::Process(service, avs, 100, 99));
    ASSERT_TRUE(client->empty());

    auto request = test::utility::CreateMessage<uxas::messages::route::RoutePlanRequest>(
        "<RoutePlanRequest Series=\"ROUTE\">"
            "<RequestID>1737948184</RequestID>"
            "<AssociatedTaskID>1000</AssociatedTaskID>"
            "<VehicleID>400</VehicleID>"
            "<OperatingRegion>0</OperatingRegion>"
            "<RouteRequests>"
                "<RouteConstraints Series=\"ROUTE\">"
                    "<RouteID>1</RouteID>"
                    "<StartLocation>"
                        "<Location3D Series=\"CMASI\">"
                            "<Latitude>45.3171307892827</Latitude>"
                            "<Longitude>-120.991285681909</Longitude>"
                            "<Altitude>700</Altitude>"
                            "<AltitudeType>MSL</AltitudeType>"
                        "</Location3D>"
                    "</StartLocation>"
                    "<StartHeading>74.6886901855469</StartHeading>"
                    "<UseStartHeading>true</UseStartHeading>"
                    "<EndLocation>"
                        "<Waypoint Series=\"CMASI\">"
                            "<Number>1</Number>"
                            "<NextWaypoint>0</NextWaypoint>"
                            "<Speed>0</Speed>"
                            "<SpeedType>Airspeed</SpeedType>"
                            "<ClimbRate>0</ClimbRate>"
                            "<TurnType>TurnShort</TurnType>"
                            "<VehicleActionList/>"
                            "<ContingencyWaypointA>0</ContingencyWaypointA>"
                            "<ContingencyWaypointB>0</ContingencyWaypointB>"
                            "<AssociatedTasks/>"
                            "<Latitude>45.3078383932067</Latitude>"
                            "<Longitude>-121.01235892126</Longitude>"
                            "<Altitude>0</Altitude>"
                            "<AltitudeType>MSL</AltitudeType>"
                        "</Waypoint>"
                    "</EndLocation>"
                    "<EndHeading>86.0940704345703</EndHeading>"
                    "<UseEndHeading>true</UseEndHeading>"
                "</RouteConstraints>"
            "</RouteRequests>"
            "<IsCostOnlyRequest>false</IsCostOnlyRequest>"
        "</RoutePlanRequest>");
    ASSERT_NE(request, nullptr);

    ASSERT_TRUE(test::service::Process(service, request, 100, 99));

    auto response = client->get<uxas::messages::route::RoutePlanResponse>();
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response->msg->getResponseID(), request->getRequestID());
}