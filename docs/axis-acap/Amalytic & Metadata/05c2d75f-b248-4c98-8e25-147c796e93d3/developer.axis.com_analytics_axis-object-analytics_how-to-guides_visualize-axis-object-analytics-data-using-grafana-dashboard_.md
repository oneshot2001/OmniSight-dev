---
url: "https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/"
title: "Visualize AXIS Object Analytics data using Grafana® dashboard | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [Metadata & Analytics](https://developer.axis.com/analytics/)
- [AXIS Object Analytics](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#)

  - [How-to guides](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#)

    - [Visualize AXIS Object Analytics data using Grafana® dashboard](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/)
    - [AXIS Object Analytics counting data](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/axis-object-analytics-counting-data/)
- [AXIS Scene Metadata](https://developer.axis.com/analytics/axis-scene-metadata/)

- [Cloud Integrations](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#)

- [Radar](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#)


- [Home page](https://developer.axis.com/)
- AXIS Object Analytics
- How-to guides
- Visualize AXIS Object Analytics data using Grafana® dashboard

On this page

# Visualize AXIS Object Analytics data using Grafana® dashboard

This how-to guide shows you how to visualize data produced by a Crossline counting scenario, in [AXIS Object Analytics](https://www.axis.com/products/axis-object-analytics), on a [Grafana®](https://grafana.com/) dashboard.

![overview](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.overview-659a7d004aafb1d322cd6d4907deab00.jpg)

_Screenshot from Grafana to the right_

It uses MQTT as a secure transport protocol from the camera to AWS. [AWS IoT Core](https://aws.amazon.com/iot-core/) (MQTT broker) receives the MQTT messages from the camera and forwards them to a time-series database ( [Amazon Timestream](https://aws.amazon.com/timestream/)) for storage and further processing. A local or cloud instance of Grafana is connected to the Amazon Timestream database, which queries the data and displays it visually in graphs and tables.

![architecture](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.architecture-1483ec925e2772cefe32424ee5d60cf2.png)

info

Some sections can be replaced or changed to get a solution that fits other use cases. For example, replace AXIS Object Analytics with some other analytics or triggers that send metadata to the AWS cloud. Or, replace the Grafana visualization dashboard with some other data visualization platform.

## Prerequisites [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#prerequisites "Direct link to Prerequisites")

- Axis camera running AXIS Object Analytics
- Access to AWS cloud services
- Local or cloud instance of Grafana

info

The Amazon Timestream service isn't available in all regions. For this setup to work, select one of its [supported regions](https://docs.aws.amazon.com/general/latest/gr/timestream.html) for all AWS services used in this how-to guide.

Click [here](https://www.axis.com/products/axis-object-analytics#compatible-products) to see AXIS Object Analytics compatible cameras.

## Workflow [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#workflow "Direct link to Workflow")

1. [Provision AWS IoT Core and Things](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#provision-aws-iot-core-and-things)
2. [Connect the camera's MQTT client to AWS IoT Core](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#connect-the-cameras-mqtt-client-to-aws-iot-core)
3. [Configure AXIS Object Analytics to send MQTT messages](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#configure-axis-object-analytics-to-send-mqtt-messages)
4. [Provision Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#provision-amazon-timestream)
5. [Route messages from AWS IoT Core to Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#route-messages-from-aws-iot-core-to-amazon-timestream)
6. [Verify data in Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#verify-data-in-amazon-timestream)
7. [Connect Grafana to Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#connect-grafana-to-amazon-timestream)
8. [Grafana dashboard examples](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#grafana-dashboard-examples)

## Provision AWS IoT Core and Things [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#provision-aws-iot-core-and-things "Direct link to Provision AWS IoT Core and Things")

Provisioning an AWS IoT Core service provides an MQTT broker to which the Axis camera can connect:

1. Sign in to the [AWS Console](https://aws.amazon.com/console/) and search for **IoT Core**.

2. Go to **Manage** \> **All devices** \> **Things** and click **Create things**.

3. Select **Create single thing** and click **Next**.

4. Enter a unique name and click **Next**.

5. On the **Configure device certificate** page, select **Auto-generate a new certificate** and click **Next**.

6. Create a new or attach an existing policy to the certificate. You're redirected to a new page if you create a new policy. For this how-to guide, create a new policy with two statements:
   - First statement
     - **Policy effect**: `Allow`
     - **Policy action**: `iot:Connect`
     - **Policy resource**: `*`
   - Second statement


     - **Policy effect**: `Allow`
     - **Policy action**: `iot:Publish`
     - **Policy resource**: `*`

warning

Not restricting the policy resource is acceptable in an exploratory phase, but applying [least-privilege permissions](https://docs.aws.amazon.com/IAM/latest/UserGuide/best-practices.html#grant-least-privilege) is a requirement before going to production.
7. Return to the previous page to attach the applicable policies and click **Create thing**.

8. Download the **Device certificate**, **Public key file**, **Private key file** and the **Root CA certificate**.
![download aws certificates](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-download-certificates-f60f4054b3a75e332a58946600031560.png)

_Screenshot from AWS Console_

9. Click **Done**.


## Connect the camera's MQTT client to AWS IoT Core [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#connect-the-cameras-mqtt-client-to-aws-iot-core "Direct link to Connect the camera's MQTT client to AWS IoT Core")

First, install the client and CA certificates on the Axis camera to enable a secure MQTT connection to the AWS IoT Core MQTT broker:

1. Log in to the Axis camera and go to **System** \> **Security**.
2. Click **Add certificate**.
3. Select **Upload a client-server certificate using a separate private key** and click **Next**.
4. Upload the client certificate (filename ends with `certificate.pem.crt`) and the private key (filename ends with `private.pem.key`), and click **Next**.
5. Click **Install** and then **Close**.
6. Click **Add certificate** again and select **Upload a CA certificate**.
7. Upload the root CA certificate ( `AmazonRootCA1.pem`).
8. Click **Next** and then **Install**.

Next, configure the camera's MQTT client:

1. In the web interface of Axis camera, go to **System** \> **MQTT**.
2. Under **Host**, enter the hostname for the IoT Core MQTT broker. You can find the hostname (endpoint) to the MQTT broker in the AWS Console under **IoT Core** \> **Settings**.
3. Under **Protocol**, select **MQTT over SSL** using default port **8883**.
4. Under **Client certificate**, select the previously uploaded client certificate.
5. Under **CA certificate**, select the previously uploaded CA certificate.
6. Select **Validate server certificate**.
7. Click **Save**.
8. Turn on **Connect**.

Here is an example of an **MQTT client** setup in an Axis camera.

![axis device mqtt client settings](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.axis-device-mqtt-client-settings-8544eb997bc89f58da80b8f7c89aa43b.png)

## Configure AXIS Object Analytics to send MQTT messages [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#configure-axis-object-analytics-to-send-mqtt-messages "Direct link to Configure AXIS Object Analytics to send MQTT messages")

1. In the camera's web interface, go to **Analytics** \> **AXIS Object Analytics** and click **Start**.
2. Open **AXIS Object Analytics**.
3. Select **Crossline counting** and click **Next**.
4. Select the type of objects you are interested in, and click **Next**.
5. Configure the virtual line and trigger direction.
6. Click **Finish**.

With the scenario created, let's configure the MQTT client.

1. Go to **System** \> **MQTT** \> **MQTT publication**.
![axis device mqtt publication settings](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.axis-device-mqtt-publication-settings-149fef348c9cc46773881e56d15c7fc2.png)

2. Select **Use default topic prefix** to include the serial number in the MQTT topic. Select **Include serial number** and **Include condition**. Make sure to clear **Include namespaces** to get an MQTT topic structure that can be handled by the AWS IoT Core rule. Click **Save**.

3. Click **Add condition** and select the scenario created earlier in AXIS Object Analytics.

4. Click **Add** to save the changes.

5. In the AWS Console, go to **IoT Core** and make sure that the MQTT communication is working.

6. Go to **Test** \> **MQTT test client**.

7. Enter `#` as **Topic filter** to see all incoming MQTT messages to the broker.

8. Click **Subscribe**. A new MQTT message is displayed every time an object passes the line configured in AXIS Object Analytics.
![aws mqtt test client](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-mqtt-test-client-e5aa14ad8deb31c0f83ab6a968cf6b3f.png)

_Screenshot from AWS Console_



MQTT payload example of a message produced by a Crossline counting scenario





```codeBlockLines_e6Vv
{
       "topic": "axis:CameraApplicationPlatform/ObjectAnalytics/Device1Scenario1",
       "timestamp": 1670518346712,
       "serial": "0123456789AB",
       "message": {
           "source": {},
           "key": {},
           "data": {
               "startTime": "2022-12-06T22:01:25Z",
               "reason": "car",
               "total": "20960",
               "totalBus": "0",
               "totalCar": "18103",
               "totalHuman": "0",
               "totalMotorcycle/bicycle": "0",
               "totalTruck": "2857",
               "totalUnknown": "0"
           }
       }
}

```









In case there aren’t any objects passing the line while testing, you can click **Test events** to manually trigger events in AXIS Object Analytics.

![axis object analytics manual trigger](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.axis-object-analytics-manual-trigger-17582dc457b523f2642bb1798f9509df.png)


## Provision Amazon Timestream [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#provision-amazon-timestream "Direct link to Provision Amazon Timestream")

1. In the AWS Console, search for the **Amazon Timestream** service.
2. Click **Create database**.
3. Select **Standard database** and enter a name for the database.
4. Click **Create database**.
5. Go to **Tables** and click **Create table**.
6. Set a data retention time for both memory and magnetic storage. For more information See [AWS documentation](https://docs.aws.amazon.com/timestream/latest/developerguide/storage.html).
![timestream retention settings](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-timestream-retention-settings-53cd73cdfbab713ded8c5bdc56e6902e.png)

_Screenshot from AWS Console_

## Route messages from AWS IoT Core to Amazon Timestream [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#route-messages-from-aws-iot-core-to-amazon-timestream "Direct link to Route messages from AWS IoT Core to Amazon Timestream")

01. Access **IoT Core** in the AWS Console.

02. Go to **Manage** \> **Message routing** \> **Rules**.

03. Click **Create rule**.

04. Enter a **Rule name** and click **Next**.

05. On the **Configure SQL statement** page, enter the following SQL query:



    SQL query





    ```codeBlockLines_e6Vv
    SELECT message.data.reason
    FROM 'axis/+/event/CameraApplicationPlatform/ObjectAnalytics/#'

    ```









    The statement selects the `reason` measurement found in the MQTT payload, sent by AXIS Object Analytics using the MQTT topic `axis/0123456789AB/event/CameraApplicationPlatform/ObjectAnalytics/Device1Scenario1`.



    Payload with reason data





    ```codeBlockLines_e6Vv
    {
        "topic": "axis:CameraApplicationPlatform/ObjectAnalytics/Device1Scenario1",
        "timestamp": 1670519332535,
        "serial": "0123456789AB",
        "message": {
            "source": {},
            "key": {},
            "data": {
                "startTime": "2022-12-06T22:01:25Z",
                "reason": "car",
                "total": "21093",
                "totalBus": "0",
                "totalCar": "18222",
                "totalHuman": "0",
                "totalMotorcycle/bicycle": "0",
                "totalTruck": "2871",
                "totalUnknown": "0"
            }
        }
    }

    ```











    info





    The MQTT topic wildcards `+` and `#` are used in the SQL statement.



- `+` matches any character between the two slashes `/+/`. Use the wildcard `+` instead of explicitly stating the MAC address to allow all cameras to route their data to the database.
- `#` at the end of the statement matches any character, including `/`. This means that the statement will match any topic structure after `/ObjectAnalytics/`. This will allow all AXIS Object Analytics scenarios to route their data to the database.

![aws iot core message routing sql statement](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-message-routing-sql-statement-2a548a7e5c43927946043615adfd1b1c.png)

_Screenshot from AWS Console_

06. Click **Next**.

07. On the **Attach rule actions** page, select **Timestream table** as **Action 1**.

08. Select the database and the table.

09. Enter a name in the **Dimensions name** field.

10. Set the **Dimension value** to `${serial}`, which in this case represents the MAC address of the camera.

11. Add a new dimension and set a **Dimension name** and the **Dimension value** to `${topic}`, which in this case represents a unique scenario in the AXIS Object Analytics application.

12. Under **IAM role**, click **Create new role** and enter a **Role name**. This automatically generates an IAM role.



    info





    For the MQTT messages to be stored in the database, it is necessary to grant the IoT Core service access to the Amazon Timestream service.





    ![aws rule action](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-rule-action-d69b9fdbbd710d4ea039d38dfd755089.png)

    _Screenshot from AWS Console_

13. Click **Next**.

14. Verify that all the information is correct and click **Create**.


## Verify data in Amazon Timestream [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#verify-data-in-amazon-timestream "Direct link to Verify data in Amazon Timestream")

1. Access **Amazon Timestream** in the AWS Console.

2. Go to **Management Tools** \> **Query editor**.

3. Enter the following query to list the 10 most recently added data points:



SQL query





```codeBlockLines_e6Vv
SELECT *
FROM <database>.<table>
ORDER BY time DESC
LIMIT 10

```









`<database>` is the name of your database and `<table>` is the name of your table.



info





If the name of your database or table contains special characters, you may need to encapsulate the name in quotation marks. For example, if the database is named `my-database` and the table is named `my-table`, the table reference is `"my-database"."my-table"`.

4. Click **Run** to execute the query.

![timestream query editor](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.aws-timestream-query-editor-1abcfeca1d3441afcfd3447e5f0f768f.png)

_Screenshot from AWS Console_


## Connect Grafana to Amazon Timestream [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#connect-grafana-to-amazon-timestream "Direct link to Connect Grafana to Amazon Timestream")

There are several ways of provisioning a Grafana instance, depending on your domain expertise or existing environments. We won't provide you with provisioning instructions. Instead, we'll give you a few different options:

- [Pull and run](https://hub.docker.com/r/grafana/grafana) a self-managed Grafana Docker image
- [Download and install](https://grafana.com/get/?tab=self-managed) a self-managed Grafana instance
- Provision a managed instance on [Grafana Cloud](https://grafana.com/grafana/)
- Provision an [Amazon Managed Grafana](https://aws.amazon.com/grafana/)

With a running Grafana instance, complete the following steps to build a dashboard:

01. In Grafana, go to **Configuration** \> **Plugins**.

02. Search for the **Amazon Timestream** plugin and click **Install**.

03. Once the plugin is installed, click **Create a Amazon Timestream data source**.

04. In AWS console, Access the **IAM** service to setup a user with the correct permissions and policy. This is done to handle the authentication from Grafana to Amazon Timestream.

05. Go to **Access management** \> **Users** and click **Add users**.

06. Enter a name and click **Next**.

07. On the **Set permissions** page, select the **Attach policies directly** option and search for the **AmazonTimestreamReadOnlyAccess** policy name. Select it and click **Next**.

08. Make sure the information is correct and then click **Create user**.

09. Select the created user and go to the **Security credentials** tab.

10. Scroll down to **Access keys** and click **Create access key**.

11. Select the **Third-party service** option and click **Next**.

12. Click **Create access key**. Take note of the **Access key** and **Secret access key** as these credentials are required when establishing the connection from Grafana to Amazon Timestream.

13. In Grafana, select **Access & secret key** as **Authentication Provider**, and enter **Access Key ID** and **Secret Access Key**.

14. In the **Default Region** field, enter the region in which you created your Amazon Timestream database.



    info





    A region is defined by its ID, but it also has a more descriptive name. For example, the region with ID `us-east-1` corresponds to `North Virginia`. If you're not sure which region to select, go to your Amazon Timestream database and in its summary in the AWS Console, and look at the **Database ARN**. The format of the ARN is `arn:aws:timestream:<region>:<account>:database/<database>`.

15. With the default regions selected, you can now select **Database**, **Table**, and **Measure**, all of which have been configured in AWS.

16. Click **Save and test** to verify that the connection has been established.

    ![grafana timestream connection settings](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.grafana-timestream-connection-settings-1684000669274d34866563f65ec4444c.png)

    _Screenshot from Grafana_

17. Once Grafana and Amazon Timestream are connected, click **Dashboards** \> **New Dashboard**.

18. Add a new panel.

19. In the **Data source** field, select **Amazon Timestream** and then ensure that the appropriate **Database**, **Table**, and **Measure** values are selected.

20. Enter an SQL query to visualize the data stored in Amazon Timestream. See the [examples](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#grafana-dashboard-examples) for queries that can provide insights using graphical representations of the data.


## Grafana dashboard examples [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#grafana-dashboard-examples "Direct link to Grafana dashboard examples")

Below are some SQL queries that you can use to visualize the object counting data produced by AXIS Object Analytics.

### Example 1 [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#example-1 "Direct link to Example 1")

Count the number of cars in the time range selected for the current dashboard. Aggregated counts in 1-hour sections.

SQL query

```codeBlockLines_e6Vv
SELECT count(*) AS Cars, BIN(time, 1h) AS bin_time
FROM $__database.$__table
WHERE measure_value::varchar = 'car' AND topic = 'axis:CameraApplicationPlatform/ObjectAnalytics/Device1Scenario1' AND serial = '0123456789AB'
GROUP BY BIN(time, 1h)
ORDER BY bin_time

```

![grafana dashboard example 1](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.grafana-dashboard-example-1-3899bd43c404c99b5fa2779e69f14482.png)

_Screenshot from Grafana_

### Example 2 [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#example-2 "Direct link to Example 2")

Count the number of cars in fixed time ranges such as the last hour, the last day, or the 7 days.

SQL query

```codeBlockLines_e6Vv
SELECT count(*) AS "Last hour"
FROM $__database.$__table
WHERE measure_value::varchar = 'car' AND time between ago(1h) and now() AND topic = 'axis:CameraApplicationPlatform/ObjectAnalytics/Device1Scenario1' AND serial = '0123456789AB'

```

![grafana dashboard example 2](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.grafana-dashboard-example-2-a78f8dcde51aa3f72e5dfa0a3a005ef3.png)

_Screenshot from Grafana_

### Inspirational dashboard [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#inspirational-dashboard "Direct link to Inspirational dashboard")

The following dashboard shows different panels that represent several object detections by AXIS Object Analytics. E.g. Humans, Cars, and Motorcycles/Bicycles in defined directions.

![grafana dashboard inspiration 1](https://developer.axis.com/assets/images/visualize-axis-object-analytics-data-using-grafana-dashboard.grafana-dashboard-inspiration-1-87dfd2aee22643e3c79e3bb6b0f64ed6.png)

_Screenshot from Grafana_

## Disclaimer [​](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/\#disclaimer "Direct link to Disclaimer")

The Grafana Labs Marks are trademarks of Grafana Labs, and are used with Grafana Labs’ permission. We are not affiliated with, endorsed or sponsored by Grafana Labs or its affiliates. Amazon Web Services, AWS and the Powered by AWS logo are trademarks of Amazon.com, Inc. or its affiliates. Docker and the Docker logo are trademarks or registered trademarks of Docker, Inc. in the United States and/or other countries. Docker, Inc. and other parties may also have trademark rights in other terms used herein. All other trademarks are the property of their respective owners, and we are not affiliated with, endorsed or sponsored by them or their affiliates.

As described in this document, you may be able to connect to, access and use third party products, web sites, example code, software or services (“Third Party Services”). You acknowledge that any such connection and access to such Third Party Services are made available to you for convenience only. Axis does not endorse any Third Party Services, nor does Axis make any representations or provide any warranties whatsoever with respect to any Third Party Services, and Axis specifically disclaims any liability or obligations with regard to Third Party Services. The Third Party Services are provided to you in accordance with their respective terms and conditions, and you alone are responsible for ensuring that you (a) procure appropriate rights to access and use any such Third Party Services and (b) comply with the terms and conditions applicable to its use.

PLEASE BE ADVISED THAT THIS DOCUMENT IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, AND IS NOT INTENDED TO, AND SHALL NOT, CREATE ANY LEGAL OBLIGATION FOR AXIS COMMUNICATIONS AB AND/OR ANY OF ITS AFFILIATES. THE ENTIRE RISK AS TO THE USE, RESULTS AND PERFORMANCE OF THIS DOCUMENT AND ANY THIRD PARTY SERVICES REFERENCED HEREIN IS ASSUMED BY THE USER OF THE DOCUMENT AND AXIS DISCLAIMS AND EXCLUDES, TO THE MAXIMUM EXTENT PERMITTED BY LAW, ALL WARRANTIES, WHETHER STATUTORY, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT AND PRODUCT LIABILITY.

Last updated on **Aug 18, 2025**

[Previous\\
\\
Metadata & Analytics](https://developer.axis.com/analytics/) [Next\\
\\
AXIS Object Analytics counting data](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/axis-object-analytics-counting-data/)

- [Prerequisites](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#prerequisites)
- [Workflow](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#workflow)
- [Provision AWS IoT Core and Things](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#provision-aws-iot-core-and-things)
- [Connect the camera's MQTT client to AWS IoT Core](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#connect-the-cameras-mqtt-client-to-aws-iot-core)
- [Configure AXIS Object Analytics to send MQTT messages](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#configure-axis-object-analytics-to-send-mqtt-messages)
- [Provision Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#provision-amazon-timestream)
- [Route messages from AWS IoT Core to Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#route-messages-from-aws-iot-core-to-amazon-timestream)
- [Verify data in Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#verify-data-in-amazon-timestream)
- [Connect Grafana to Amazon Timestream](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#connect-grafana-to-amazon-timestream)
- [Grafana dashboard examples](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#grafana-dashboard-examples)
  - [Example 1](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#example-1)
  - [Example 2](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#example-2)
  - [Inspirational dashboard](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#inspirational-dashboard)
- [Disclaimer](https://developer.axis.com/analytics/axis-object-analytics/how-to-guides/visualize-axis-object-analytics-data-using-grafana-dashboard/#disclaimer)

COMMUNITY

- [Axis Developer Community](https://axis.com/developer-community)
- [Axis on GitHub](https://github.com/AxisCommunications)

SOCIAL

- [LinkedIn](https://www.linkedin.com/company/axis-communications)
- [YouTube](https://www.youtube.com/@AxisCommunications)

LEGAL AND COMPLIANCE

- [Legal](https://www.axis.com/legal)
- [Privacy](https://www.axis.com/privacy)

© 2025 Axis Communications AB. All rights reserved.