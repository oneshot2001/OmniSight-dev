---
url: "https://developer.axis.com/vapix/physical-access-control/"
title: "Physical access control | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/vapix/physical-access-control/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [VAPIX®](https://developer.axis.com/vapix/)
- [Applications](https://developer.axis.com/vapix/applications/)

- [Audio systems](https://developer.axis.com/vapix/audio-systems/)

- [Body worn systems](https://developer.axis.com/vapix/body-worn-systems/)
- [Device configuration](https://developer.axis.com/vapix/device-configuration/)

- [Intercom](https://developer.axis.com/vapix/intercom/)

- [Network video](https://developer.axis.com/vapix/network-video/)

- [Physical access control](https://developer.axis.com/vapix/physical-access-control/)

  - [Access control service](https://developer.axis.com/vapix/physical-access-control/access-control-service/)
  - [Connection service](https://developer.axis.com/vapix/physical-access-control/connection-service/)
  - [Door control service](https://developer.axis.com/vapix/physical-access-control/door-control-service/)
  - [Event logger service](https://developer.axis.com/vapix/physical-access-control/event-logger-service/)
  - [IdPoint service](https://developer.axis.com/vapix/physical-access-control/idpoint-service/)
  - [I/O assignment service](https://developer.axis.com/vapix/physical-access-control/io-assignment-service/)
  - [Keystore service](https://developer.axis.com/vapix/physical-access-control/keystore-service/)
  - [Peripherals](https://developer.axis.com/vapix/physical-access-control/peripherals/)
  - [Schedule service](https://developer.axis.com/vapix/physical-access-control/schedule-service/)
  - [System integration examples](https://developer.axis.com/vapix/physical-access-control/system-integration-examples/)
  - [Third party credential service](https://developer.axis.com/vapix/physical-access-control/third-party-credential-service/)
  - [User service](https://developer.axis.com/vapix/physical-access-control/user-service/)
- [Radar](https://developer.axis.com/vapix/radar/)


- [Home page](https://developer.axis.com/)
- Physical access control

On this page

# Physical access control

VAPIX® Physical Access Control APIs is a set of services for configuration and management of access control systems. An access control system consists of Axis network door controllers and connected equipment such as doors, locks and readers.

The access control API documentation describes the different services and provides numerous examples how to use the APIs in common scenarios. Examples are available in JSON and SOAP. Use the buttons to switch between JSON and SOAP.

By design, the access control APIs are flexible. In most cases, there are several different ways to configure the system to perform a specific task. This makes it possible to develop workflows that suit different needs. The examples in the documentation do not describe all possible configuration options. Check the API specifications for complete lists of available settings and options.

As inspiration, sections [Entry manager](https://developer.axis.com/vapix/physical-access-control/system-integration-examples/#entry-manager) and [Entry manager examples](https://developer.axis.com/vapix/physical-access-control/system-integration-examples/#entry-manager-examples) show how AXIS Entry Manager uses the access control APIs to set up and manage an access control system.

## Version history [​](https://developer.axis.com/vapix/physical-access-control/\#version-history "Direct link to Version history")

| Date | Updates |
| --- | --- |
| 2024–11–01 | [IdPoint service](https://developer.axis.com/vapix/physical-access-control/idpoint-service/): Added example for multidrop setup for OSDP readers. |
| 2022–05–19 | [IdPoint service](https://developer.axis.com/vapix/physical-access-control/idpoint-service/): Added circuit information. |
| 2021–10–06 | [Peripherals](https://developer.axis.com/vapix/physical-access-control/peripherals/): Minor updates. |
| 2020–11–10 | [Access control service](https://developer.axis.com/vapix/physical-access-control/access-control-service/): Minor updates. |
| 2020–08–27 | [Peripherals](https://developer.axis.com/vapix/physical-access-control/peripherals/): Minor updates. |
| 2020–04–28 | [Introduction](https://developer.axis.com/vapix/physical-access-control/): Minor updates. |
| 2020–04–02 | [Entry manager examples](https://developer.axis.com/vapix/physical-access-control/system-integration-examples/#entry-manager-examples): Minor updates. |
| 2019–04–25 | [Access control service guide](https://developer.axis.com/vapix/physical-access-control/access-control-service/#access-control-service-guide): Minor updates. [Door control service guide](https://developer.axis.com/vapix/physical-access-control/door-control-service/#door-control-service-guide): Minor updates. |
| 2018–10–16 | Rearranged order of the folder topics. Merged Entry manager and Entry manager examples into [System integration examples](https://developer.axis.com/vapix/physical-access-control/system-integration-examples/) . |
| 2018–08-03 | Added Remote reader information to [IdPoint service](https://developer.axis.com/vapix/physical-access-control/idpoint-service/) and [Peripherals](https://developer.axis.com/vapix/physical-access-control/peripherals/). |
| 2018–04–24 | Added Whitelist information for [SmartIntego doors](https://developer.axis.com/vapix/physical-access-control/peripherals/#smartintego-doors). |
| 2017–08–18 | Added section [SmartIntego doors](https://developer.axis.com/vapix/physical-access-control/peripherals/#smartintego-doors). Added section [Third party credential service guide](https://developer.axis.com/vapix/physical-access-control/third-party-credential-service/#third-party-credential-service-guide). |
| 2017–02–24 | Added section [Enable anti-passback](https://developer.axis.com/vapix/physical-access-control/access-control-service/#enable-anti-passback). Added section [Keystore service](https://developer.axis.com/vapix/physical-access-control/keystore-service/). |
| 2016–10–14 | Added API specifications. Moved documentation to VAPIX®. |
|  | Added credential changed and credential removed events. |
|  | Changed namespaces for continued ONVIF conformance. [See separate integration guide](https://www.axis.com/files/manuals/Integration_guide_name_space_change_160707.pdf). |
|  | Added section [Elevator access control](https://developer.axis.com/vapix/physical-access-control/peripherals/#elevator-access-control). |
|  | Added `FetchEvents2` and `FetchAlarms2`. Chapter [Event logger service](https://developer.axis.com/vapix/physical-access-control/event-logger-service/). |
|  | Added remove device connection status event. |
|  | Added new door monitor and REX parameters. |
|  | Updated battery and radio disturbance events. |
|  | Added function `GetCredentialStatistics`. Section [Credential statistics](https://developer.axis.com/vapix/physical-access-control/access-control-service/#credential-statistics). |
|  | Moved AperioTM sections to new chapter [Peripherals](https://developer.axis.com/vapix/physical-access-control/peripherals/). |
|  | Added `BitCount` field in notification topic `tns1:IdPoint/tnsaxis:Request/IdData`. |
|  | Updated AperioTM door description. |
|  | Information about AperioTM doors added. |
|  | Door monitor supervised inputs options corrected. Table in section [Door configuration](https://developer.axis.com/vapix/physical-access-control/door-control-service/#door-configuration-guide). |
|  | List of notification topics added. Section [Notification topics](https://developer.axis.com/vapix/physical-access-control/event-logger-service/#notification-topics). |
|  | Example how to use door priorities and schedules added. Section [Door priorities and door schedules](https://developer.axis.com/vapix/physical-access-control/door-control-service/#door-priorities-and-door-schedules). |
|  | Access Control Service updated with disable option for `AccessPoint`. |
|  | Event Logger Service updated with disable option for global distribution. |
|  | Duress access added. |
|  | Supervised inputs added. |
|  | Extended description of Connection Service. |
|  | This version table added. |
|  | AXIS Entry Manager I/O Assignment table corrected. |
|  | Access Control Service updated with regards on strict id data order. |
|  | Document released. |

## Prerequisites [​](https://developer.axis.com/vapix/physical-access-control/\#prerequisites "Direct link to Prerequisites")

### Identification [​](https://developer.axis.com/vapix/physical-access-control/\#identification "Direct link to Identification")

Use VAPIX® Entry Service API to check if the API is supported. Services and namespaces are listed in the following table.

Supported services and namespaces

| Service | Namespace for SOAP request | Placeholder for SOAP requests | **VAPIX ® services for JSON request** | Supported Axis network door controllers |
| --- | --- | --- | --- | --- |
| Connection service | [http://www.axis.com/vapix/ws/connection](http://www.axis.com/vapix/ws/connection) | `aconn` | `/vapix/aconn` | A1001 |
| Door control service | [http://www.onvif.org/ver10/doorcontrol/wsdl](http://www.onvif.org/ver10/doorcontrol/wsdl) | `tdc` | `/vapix/doorcontrol` | A1001, A1210, A1601, A1610 |
|  | [http://www.axis.com/vapix/ws/DoorControl](http://www.axis.com/vapix/ws/DoorControl) | `axtdc` | `/vapix/doorcontrol` | A1001, A1210, A1601, A1610 |
| Event logger service | [http://www.axis.com/vapix/ws/EventLogger](http://www.axis.com/vapix/ws/EventLogger) | `axlog` | `/vapix/eventlogger` | A1001, A1210, A1601, A1610 |
|  | [http://www.axis.com/vapix/ws/EventLoggerConfig](http://www.axis.com/vapix/ws/EventLoggerConfig) | `axloc` | `/vapix/eventlogger` | A1001, A1210, A1601, A1610 |
| IdPoint service | [http://www.axis.com/vapix/ws/IdPoint](http://www.axis.com/vapix/ws/IdPoint) | `axtid` | `/vapix/idpoint` | A1001, A1210, A1601, A1610 |
| I/O assignment service | [http://www.axis.com/vapix/ws/AxisIo](http://www.axis.com/vapix/ws/AxisIo) | `axisio` | `/vapix/pin` | A1001, A1210, A1601, A1610 |
| AccessControl service | [http://www.onvif.org/ver10/accesscontrol/wsdl](http://www.onvif.org/ver10/accesscontrol/wsdl) | `tac` | `/vapix/pacs` | A1001, A1210, A1601, A1610 |
|  | [http://www.axis.com/vapix/ws/pacs](http://www.axis.com/vapix/ws/pacs) | `pacsaxis` | `/vapix/pacs` | A1001, A1210, A1601, A1610 |
| Schedule service | [http://www.axis.com/vapix/ws/schedule](http://www.axis.com/vapix/ws/schedule) | `axsch` | `/vapix/schedule` | A1001, A1210, A1601, A1610 |
| User service | [http://www.axis.com/vapix/ws/user](http://www.axis.com/vapix/ws/user) | `axudb` | `/vapix/pacs` | A1001, A1210, A1601, A1610 |
| Service discovery service | [http://www.axis.com/vapix/ws/AxisServiceDiscovery](http://www.axis.com/vapix/ws/AxisServiceDiscovery) | `axissd` | `/vapix/axissd` | A1001, A1210, A1601, A1610 |
| Remote I/O service | [http://www.axis.com/vapix/ws/axrio](http://www.axis.com/vapix/ws/axrio) | `axrio` | `/vapix/axrio` | A1001, A1210, A1601, A1610 |
| Keystore service | [http://www.axis.com/vapix/ws/KeyStore](http://www.axis.com/vapix/ws/KeyStore) | `axkey` | `/vapix/axkey` | A1001, A1210, A1601, A1610 |
| Third party credential service | [http://www.axis.com/vapix/ws/thirdpartycredential](http://www.axis.com/vapix/ws/thirdpartycredential) | `axtpc` | `/vapix/axtpc` | A1001 |

Supported events, action templates and recipient templates are product dependent.

## Administrative users [​](https://developer.axis.com/vapix/physical-access-control/\#administrative-users "Direct link to Administrative users")

The existing administrative user interfaces are `/axis-cgi/pwdgrp.cgi` for VAPIX® specific users and `/onvif/device_service` for ONVIF specific users. For usage details, refer to the specific documentation for each interface.

### Administrative user distribution [​](https://developer.axis.com/vapix/physical-access-control/\#administrative-user-distribution "Direct link to Administrative user distribution")

The administrative user distribution (AUD) is an underlying layer to the existing Administrative User interfaces.

Actions on AUD, that is, add, update and remove, will be propagated through the system, if having multiple door controllers in a network as described in section [Connection service](https://developer.axis.com/vapix/physical-access-control/connection-service/), and the accounts will be available on all peers in the door controller network.

Administrative users have three levels of rights (viewer, operator and admin) and where admin level is required for all actions. The lower levels exist but as they can not execute any actions they are not fully supported by AUD.

### Actions on multiple ONVIF<sup>TM</sup> administrative users [​](https://developer.axis.com/vapix/physical-access-control/\#actions-on-multiple-onvifsuptmsup-administrative-users "Direct link to Actions on multiple ONVIF<sup>TM</sup> administrative users")

When executing actions on multiple ONVIF administrative users, the execution is faster when the administrative users are given in a batch in one call than given one by one in sequence.

### Default administrator account and password [​](https://developer.axis.com/vapix/physical-access-control/\#default-administrator-account-and-password "Direct link to Default administrator account and password")

The door controller has the following default administrator account:

- **User name**: `root`
- **Password**: `pass`

For security reasons, it is strongly recommended to change the password. Change the password, preferably using HTTPS, when logging in to AXIS Entry Manager for the first time, or by using VAPIX® General System Settings API.

## Overview [​](https://developer.axis.com/vapix/physical-access-control/\#overview "Direct link to Overview")

The physical access control APIs consist of different services, for example access control service, door control service and schedule service. Each service provides a number of tightly coupled interfaces. The services and interfaces are constructed similarly and have many common features. This chapter discusses the concepts that are common to all services and describes the syntax used in the code examples in the documentation.

### Administrators and users [​](https://developer.axis.com/vapix/physical-access-control/\#administrators-and-users "Direct link to Administrators and users")

The access control system (aka PACS system) has different types of users, for example:

1. User. A PACS user, also known as credential holder or card holder. This user is a physical person who uses the system and the hardware to get access to doors in rooms and buildings.

2. Administrator. A system administrator who uses AXIS Entry Manager to manage the access control system, its door controllers, connected devices, users, and schedules. This user always belongs to the Administrator user group.

3. PACS API user. A client such as an access management system that makes VAPIX® or ONVIF calls to the door controller.


### Code examples [​](https://developer.axis.com/vapix/physical-access-control/\#code-examples "Direct link to Code examples")

The code examples in the API documentation are available in JavaScript Object Notation (JSON) and in SOAP. Use the buttons to switch between JSON and SOAP.

#### JSON examples [​](https://developer.axis.com/vapix/physical-access-control/\#json-examples "Direct link to JSON examples")

JSON code examples have the following syntax:

```codeBlockLines_e6Vv
{
    "AccessController": [\
        {\
            "AccessPoint": [""],\
            "Description": "",\
            "Name": "Axis-00408c184bdb AccessController",\
            "token": "Axis-00408c184bdb AccessController"\
        }\
    ]
}

```

An API request to the door controller should be sent to the correct service and should include the request function as well as the data structures required by the request. Many of the code examples in VAPIX® only show the data structures and not the complete request. The service and request function must then be added before executing the request. As an example, consider the example from section [Retrieve default configuration](https://developer.axis.com/vapix/physical-access-control/access-control-service/#retrieve-default-configuration). The corresponding request and response looks like:

Request

```codeBlockLines_e6Vv
{
    "pacsaxis:GetAccessControllerList": {}
}

```

Response

```codeBlockLines_e6Vv
{
    "AccessController": [\
        {\
            "token": "Axis-00408c184bdb AccessController",\
            "Name": "Axis-00408c184bdb AccessController",\
            "Description": "",\
            "AccessPoint": []\
        }\
    ]
}

```

The code examples can be executed using command-line tools such as cURL or the Invoke-RestMethod in Microsoft Windows® PowerShell. The following example shows how to use cURL to execute the request above:

```codeBlockLines_e6Vv
$ curl --anyauth "http://<user>:<password>@<ip>/vapix/pacs" -s -d ’{"pacsaxis:GetAccessControllerList":{}}’
> {
>   "AccessController":
>     [\
>       {\
>         "token": "Axis-00408c184bdb AccessController",\
>         "Name": "Axis-00408c184bdb AccessController",\
>         "Description": "",\
>         "AccessPoint": []\
>       }\
>     ]
> }

```

`<user>, <password>` and `<ip>` should be replaced by the user name, password and IP address used by the door controller.

`/vapix/pacs` is the VAPIX® service to use for the `AccessControl` service. In JSON requests, different VAPIX® services should be used for the different door controller services.

#### SOAP examples [​](https://developer.axis.com/vapix/physical-access-control/\#soap-examples "Direct link to SOAP examples")

Most of the SOAP code examples in the documentation show the SOAP envelope body (that is, `<SOAP-ENV:Body>`) and not the complete request. Before executing the examples, the client should create a well-formed SOAP message in XML and include the example in the message. The following example shows a part of a SOAP envelope body:

```codeBlockLines_e6Vv
<pacsaxis:AccessController token="Axis-00408c184bdb AccessController">
    <pacsaxis:Name>Axis-00408c184bdb AccessController</pacsaxis:Name>
    <pacsaxis:AccessPoint />
    <pacsaxis:Description />
</pacsaxis:AccessController>

```

The code examples can be executed using command-line tools such as cURL or the Invoke-RestMethod in Microsoft Windows® PowerShell. The following example shows how to use cURL to execute the request above. The file `GetAccessControllerList.soap` contains a well-formed SOAP message.

```codeBlockLines_e6Vv
$ cat GetAccessControllerList.soap
> <?xml version="1.0" encoding="UTF-8"?>
> <SOAP-ENV:Envelope ...>
> <SOAP-ENV:Header>
> </SOAP-ENV:Header>
> <SOAP-ENV:Body>
>   <pacsaxis:GetAccessControllerList/>
> </SOAP-ENV:Body>
> </SOAP-ENV:Envelope>
$ curl --anyauth "http://<user>:<password>@<ip>/vapix/services" -s -d@GetAccessControllerList.soap
> <?xml version="1.0" encoding="UTF-8"?>
> <SOAP-ENV:Envelope ...>
> <SOAP-ENV:Header>
> </SOAP-ENV:Header>
> <SOAP-ENV:Body>
>   <pacsaxis:GetAccessControllerListResponse>
>     <pacsaxis:AccessController token="Axis-00408c184bdb AccessController">
>       <pacsaxis:Name>Axis-00408c184bdb AccessController</pacsaxis:Name>
>       <pacsaxis:Description></pacsaxis:Description>
>     </pacsaxis:AccessController>
>   </pacsaxis:GetAccessControllerListResponse>
> </SOAP-ENV:Body>

```

To keep the example short, the namespace definitions in the `Envelope` tag are not shown here. `<user>, <password>` and `<ip>` should be replaced by the user name, password and IP address used by the door controller. The VAPIX® service for all SOAP requests is `/vapix/services`.

### Data structures [​](https://developer.axis.com/vapix/physical-access-control/\#data-structures "Direct link to Data structures")

#### Common data fields [​](https://developer.axis.com/vapix/physical-access-control/\#common-data-fields "Direct link to Common data fields")

Most of the data structures in the access control APIs have the following data fields: `token`, `Name` and `Description`.

The `token` field is required and contains a token that is used to identify the data structure. Data structures of the same type must have unique tokens. Data structures of different types can use identical tokens. When setting up a new data structure, a unique token is generated and returned to the caller if the `token` field is left empty.

In some cases, two data structure types in separate interfaces in the same service represent the same physical device. In these cases, the `token` works as a link between the two data types. For example, an access controller is represented by `pacsaxis:AccessController` and by `pacsaxis:AccessControllerConfiguration`.

The `token` is also used when data structures refer to each other. For example, if a door uses a schedule, the `token` is the link between the door and the schedule.

The `Name` and `Description` fields are optional and contain human-readable information about the data structure.

#### Common data structure types [​](https://developer.axis.com/vapix/physical-access-control/\#common-data-structure-types "Direct link to Common data structure types")

An entity in the access control APIs is usually represented by two data structures: one basic data structure and one information data structure. These are named according to the pattern `DataType` and `DataTypeInfo`, for example `AccessPoint` and `AccessPointInfo`. The information data structure `DataTypeInfo` represents the informative parts of data and contains a smaller subset of the data fields. In addition, the `DataTypeInfo` may contain a `Capabilities` data field which is not included in the `DataType`. The API provides one get request for the basic data type and one get request for the information data type. The following example shows the result of the get requests for `AccessPointInfo` and `AccessPoint`.

```codeBlockLines_e6Vv
{
    "AccessPointInfo": [\
        {\
            "token": "Axis-00408c184bdb:1353665591.412364000",\
            "Name": "Door",\
            "Description": "",\
            "AreaFrom": "",\
            "AreaTo": "",\
            "EntityType": "axtdc:Door",\
            "Entity": "Axis-00408c184bdb:1353665586.846691000",\
            "Capabilities": {\
                "DisableAccessPoint": true,\
                "AccessTaken": true\
            }\
        }\
    ]
}

```

```codeBlockLines_e6Vv
<pacsaxis:AccessPointInfo token="Axis-00408c184bdb:1353665591.412364000">
    <pacsaxis:Name>Door</pacsaxis:Name>
    <pacsaxis:Description />
    <pacsaxis:AreaFrom />
    <pacsaxis:AreaTo />
    <pacsaxis:EntityType>axtdc:Door</pacsaxis:EntityType>
    <pacsaxis:Entity>Axis-00408c184bdb:1353665586.846691000</pacsaxis:Entity>
    <pacsaxis:Capabilities AccessTaken="true" DisableAccessPoint="true" />
</pacsaxis:AccessPointInfo>

```

```codeBlockLines_e6Vv
{
    "AccessPoint": [\
        {\
            "token": "Axis-00408c184bdb:1353665591.412364000",\
            "Name": "Door",\
            "Description": "",\
            "AreaFrom": "",\
            "AreaTo": "",\
            "EntityType": "axtdc:Door",\
            "Entity": "Axis-00408c184bdb:1353665586.846691000",\
            "Enabled": true,\
            "DoorDeviceUUID": "5581ad80-95b0-11e0-b883-00408c184bdb",\
            "IdPointDevice": [\
                {\
                    "IdPoint": "Axis-00408c184bdb:1353665586.923475000",\
                    "DeviceUUID": ""\
                }\
            ],\
            "AuthenticationProfile": ["Axis-00408c184bdb:1353665638.159357000"],\
            "Attribute": [\
                {\
                    "type": "",\
                    "Name": "Direction",\
                    "Value": "in"\
                }\
            ],\
            "ActionArgument": [],\
            "Action": "Access"\
        }\
    ]
}

```

```codeBlockLines_e6Vv
<pacsaxis:AccessPoint token="Axis-00408c184bdb:1353665591.412364000">
    <pacsaxis:Name />
    <pacsaxis:Description />
    <pacsaxis:AreaFrom />
    <pacsaxis:AreaTo />
    <pacsaxis:EntityType>axtdc:Door</pacsaxis:EntityType>
    <pacsaxis:Entity>Axis-00408c184bdb:1353665586.846691000</pacsaxis:Entity>
    <pacsaxis:Enabled>true</pacsaxis:Enabled>
    <pacsaxis:DoorDeviceUUID>5581ad80-95b0-11e0-b883-00408c184bdb</pacsaxis:DoorDeviceUUID>
    <pacsaxis:IdPointDevice>
        <pacsaxis:IdPoint>Axis-00408c184bdb:1353665586.923475000</pacsaxis:IdPoint>
        <pacsaxis:DeviceUUID />
    </pacsaxis:IdPointDevice>
    <pacsaxis:Attribute Name="Direction" Value="out" type="" />
    <pacsaxis:Action>Access</pacsaxis:Action>
</pacsaxis:AccessPoint>

```

For more information about get requests, see [Set requests](https://developer.axis.com/vapix/physical-access-control/#set-requests).

### Set requests [​](https://developer.axis.com/vapix/physical-access-control/\#set-requests "Direct link to Set requests")

Set requests are used to add new data items as well as to replace existing items. If the `token` field is left empty or if the provided `token` does not exist, the set request will create a new data structure on the door controller. If the provided `token` exists, data in the existing data structure will be replaced by the data provided in the set request. The response to a set request is a list of existing tokens.

As discussed in section [Common data structure types](https://developer.axis.com/vapix/physical-access-control/#common-data-structure-types), most entities are represented by two data types. A set request will create or replace both data types as both are required.

### Get requests [​](https://developer.axis.com/vapix/physical-access-control/\#get-requests "Direct link to Get requests")

Get requests are used to retrieve data from the door controller. The response is a list of available data items. Data elements can, in most cases, be retrieved either as a single data item or as a list of data items.

A single data item is retrieved by specifying the token in a get request named according to the pattern `Get<DataType>`.

A data list can be retrieved using `Get<DataType>List` or `Get<DataType>`.

If no parameters are specified, `Get<DataType>List` returns a list of all available data items. To limit the output, the following parameters can be used:

- `Limit:` Maximum number of returned data elements.
- `StartReference:` This parameter is used to continue fetching data from the last position and enables iterations over large data sets in smaller chunks. A client shall not make any assumptions on `StartReference` contents and shall always pass the `NextStartReference` value returned from a previous request to continue fetching data. A reference should only be used once.

If the full data set can not be returned in one call or if `Limit` is specified, a `NextStartReference` will be returned together with the data chunk. The `NextStartReference` can be used to continue fetching data from the last point.

`Get<DataType>` returns a list of data types matching the tokens of the supplied list of tokens. The input parameter called `Token` will be used to filter returned elements, but, if left empty, no elements will be returned.

Retrieving lists with too many data items might affect performance. It is recommended to use the parameters to limit the output.

### Events [​](https://developer.axis.com/vapix/physical-access-control/\#events "Direct link to Events")

The door controller will dispatch events. By creating pull-point subscriptions, it is possible to listen to events in real time. Refer to the ONVIF documentation for details.

The Event Logger Service provides logging of events, see section [Event logger service](https://developer.axis.com/vapix/physical-access-control/event-logger-service/). Event examples in the documentation are written in the format received when using the API request `axlog:FetchEvents`.

### UUID [​](https://developer.axis.com/vapix/physical-access-control/\#uuid "Direct link to UUID")

The Universally Unique Identifer (UUID) is used to identify door controllers in a network. Each door controller has a unique UUID. In data structures that represent configurations for a specific door controller, for example, ID point configuration, door configuration and access controller configuration, the UUID is a data field.

If the UUID data field is left empty, the UUID of the door controller to which the request is sent will be inserted automatically. That is, as long as configurations are sent to the intended door controller, the UUID does not need to be specified.

If several door controllers are connected in a door controller network, requests to get or set configurations on one door controller can be sent to any door controller in the network as long as the UUID is specified. The configuration will be read from or set to the door controller that matches the specified UUID.

To find the UUID:s of the door controllers in the network, use `aconn:GetPeerList` from the Connection Service, see [Connection service](https://developer.axis.com/vapix/physical-access-control/connection-service/). The following example shows the request and response. The response lists all door controllers in the network, including the door controller to which the request was sent.

```codeBlockLines_e6Vv
curl --anyauth "http://<user>:<password>@<ip>/vapix/aconn" -s
    -d’{"aconn:GetPeerList":{}}’

{
  "Self": {
    "ID": "00408C184BDB",
    "ExpectedAddress": "",
    "LastKnownAddress": "",
    "DeviceUUID": "5581ad80-95b0-11e0-b883-00408c184bdb",
    "TrustState": "InPeerNetwork",
    "ConnectionState": "Connected" },
  "Peer": [ {\
    "ID": "00408C184BFA",\
    "ExpectedAddress": "",\
    "LastKnownAddress": "172.25.9.115",\
    "DeviceUUID": "5581ad80-95b0-11e0-b883-00408c184bfa",\
    "TrustState": "InPeerNetwork",\
    "ConnectionState": "Connected" } ]
}

```

```codeBlockLines_e6Vv
$ cat GetPeerList.soap
> <?xml version="1.0" encoding="UTF-8"?>
> <SOAP-ENV:Envelope ...>
> <SOAP-ENV:Header>
> </SOAP-ENV:Header>
> <SOAP-ENV:Body>
>   <aconn:GetPeerList/>
> </SOAP-ENV:Body>
> </SOAP-ENV:Envelope>

$ curl --anyauth "http://<user>:<password>@<ip>/vapix/services" -s -d@GetPeerList.soap
> <SOAP-ENV:Envelope ...>
> <SOAP-ENV:Header>
> </SOAP-ENV:Header>
> <SOAP-ENV:Body>
>   <aconn:GetPeerListResponse>
>     <aconn:Self>
>       <aconn:ID>00408C184BDB</aconn:ID>
>       <aconn:ExpectedAddress></aconn:ExpectedAddress>
>       <aconn:LastKnownAddress></aconn:LastKnownAddress>
>       <aconn:DeviceUUID>5581ad80-95b0-11e0-b883-00408c184bdb</aconn:DeviceUUID>
>       <aconn:TrustState>InPeerNetwork</aconn:TrustState>
>       <aconn:ConnectionState>Connected</aconn:ConnectionState>
>     </aconn:Self>
>     <aconn:Peer>
>       <aconn:ID>00408C184BFA</aconn:ID>
>       <aconn:ExpectedAddress></aconn:ExpectedAddress>
>       <aconn:LastKnownAddress>172.25.9.115</aconn:LastKnownAddress>
>       <aconn:DeviceUUID>5581ad80-95b0-11e0-b883-00408c184bfa</aconn:DeviceUUID>
>       <aconn:TrustState>InPeerNetwork</aconn:TrustState>
>       <aconn:ConnectionState>Connected</aconn:ConnectionState>
>     </aconn:Peer>
>   </aconn:GetPeerListResponse>
> </SOAP-ENV:Body>
> </SOAP-ENV:Envelope>

```

Last updated on **Aug 28, 2025**

[Previous\\
\\
Zipstream technology](https://developer.axis.com/vapix/network-video/zipstream-technology/) [Next\\
\\
Access control service](https://developer.axis.com/vapix/physical-access-control/access-control-service/)

- [Version history](https://developer.axis.com/vapix/physical-access-control/#version-history)
- [Prerequisites](https://developer.axis.com/vapix/physical-access-control/#prerequisites)
  - [Identification](https://developer.axis.com/vapix/physical-access-control/#identification)
- [Administrative users](https://developer.axis.com/vapix/physical-access-control/#administrative-users)
  - [Administrative user distribution](https://developer.axis.com/vapix/physical-access-control/#administrative-user-distribution)
  - [Actions on multiple ONVIF<sup>TM</sup> administrative users](https://developer.axis.com/vapix/physical-access-control/#actions-on-multiple-onvifsuptmsup-administrative-users)
  - [Default administrator account and password](https://developer.axis.com/vapix/physical-access-control/#default-administrator-account-and-password)
- [Overview](https://developer.axis.com/vapix/physical-access-control/#overview)
  - [Administrators and users](https://developer.axis.com/vapix/physical-access-control/#administrators-and-users)
  - [Code examples](https://developer.axis.com/vapix/physical-access-control/#code-examples)
  - [Data structures](https://developer.axis.com/vapix/physical-access-control/#data-structures)
  - [Set requests](https://developer.axis.com/vapix/physical-access-control/#set-requests)
  - [Get requests](https://developer.axis.com/vapix/physical-access-control/#get-requests)
  - [Events](https://developer.axis.com/vapix/physical-access-control/#events)
  - [UUID](https://developer.axis.com/vapix/physical-access-control/#uuid)

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