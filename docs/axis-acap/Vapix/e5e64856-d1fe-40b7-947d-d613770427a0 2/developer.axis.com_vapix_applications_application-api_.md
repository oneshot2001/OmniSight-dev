---
url: "https://developer.axis.com/vapix/applications/application-api/"
title: "Application API | Axis developer documentation"
---

[Skip to main content](https://developer.axis.com/vapix/applications/application-api/#__docusaurus_skipToContent_fallback)

[![Axis Communications Logo](https://developer.axis.com/img/axis-logo.svg)\\
**Axis developer documentation**](https://developer.axis.com/)

```

```

[Blog](https://developer.axis.com/blog/)

- [VAPIX®](https://developer.axis.com/vapix/)
- [Applications](https://developer.axis.com/vapix/applications/)

  - [Application API](https://developer.axis.com/vapix/applications/application-api/)
  - [Application configuration API](https://developer.axis.com/vapix/applications/application-configuration-api/)
  - [AXIS Object analytics API](https://developer.axis.com/vapix/applications/axis-object-analytics-api/)
  - [Cross line detection 1.1 API](https://developer.axis.com/vapix/applications/cross-line-detection-1.1-api/)
  - [Demographic identifier API](https://developer.axis.com/vapix/applications/demographic-identifier-api/)
  - [Digital autotracking API](https://developer.axis.com/vapix/applications/digital-autotracking-api/)
  - [Fence guard](https://developer.axis.com/vapix/applications/fence-guard/)
  - [License plate verifier API](https://developer.axis.com/vapix/applications/license-plate-verifier-api/)
  - [Loitering guard](https://developer.axis.com/vapix/applications/loitering-guard/)
  - [Motion guard](https://developer.axis.com/vapix/applications/motion-guard/)
  - [P8815-2 3D people counter API](https://developer.axis.com/vapix/applications/p8815-2-3d-people-counter-api/)
  - [People counter API](https://developer.axis.com/vapix/applications/people-counter-api/)
  - [Queue monitor API](https://developer.axis.com/vapix/applications/queue-monitor-api/)
  - [Video motion detection 2.1 API](https://developer.axis.com/vapix/applications/video-motion-detection-2.1-api/)
  - [Video motion detection 3 API](https://developer.axis.com/vapix/applications/video-motion-detection-3-api/)
  - [Video motion detection 4 API](https://developer.axis.com/vapix/applications/video-motion-detection-4-api/)
- [Audio systems](https://developer.axis.com/vapix/audio-systems/)

- [Body worn systems](https://developer.axis.com/vapix/body-worn-systems/)
- [Device configuration](https://developer.axis.com/vapix/device-configuration/)

- [Intercom](https://developer.axis.com/vapix/intercom/)

- [Network video](https://developer.axis.com/vapix/network-video/)

- [Physical access control](https://developer.axis.com/vapix/physical-access-control/)

- [Radar](https://developer.axis.com/vapix/radar/)


- [Home page](https://developer.axis.com/)
- [Applications](https://developer.axis.com/vapix/applications/)
- Application API

On this page

# Application API

## Description [​](https://developer.axis.com/vapix/applications/application-api/\#description "Direct link to Description")

Use VAPIX® Application API to upload, control and manage AXIS Camera Application Platform (ACAP) applications and their license keys.

Supported functionality:

- Upload applications to the Axis product. See [Upload application](https://developer.axis.com/vapix/applications/application-api/#upload-application).
- Start, stop, restart and remove applications. See [Control application](https://developer.axis.com/vapix/applications/application-api/#control-application).
- Configure settings usable by all applications. See [Configure applications](https://developer.axis.com/vapix/applications/application-api/#configure-applications).
- Upload, install and remove license keys. See [Manage license keys](https://developer.axis.com/vapix/applications/application-api/#manage-license-keys).
- List installed applications. See [List installed applications](https://developer.axis.com/vapix/applications/application-api/#list-installed-applications).

### Identification [​](https://developer.axis.com/vapix/applications/application-api/\#identification "Direct link to Identification")

VAPIX® Application API is supported if:

- **Property**: `Properties.EmbeddedDevelopment.Version` exists.

`list.cgi` requires:

- **Property**: `Properties.EmbeddedDevelopment.Version=1.20` and later.

`config.cgi` requires:

- **AXIS OS**: 11.2 or later

## Common examples [​](https://developer.axis.com/vapix/applications/application-api/\#common-examples "Direct link to Common examples")

These examples demonstrate how to use the Application API. Text marked in **bold** should be replaced by application-specific values.

Upload an application to the Axis product.

Request:

```codeBlockLines_e6Vv
POST /axis-cgi/applications/upload.cgi HTTP/1.1
Content-Type: multipart/form-data; boundary=fileboundary
Content-Length: <content-length>

fileboundary
Content-Disposition: form-data; name="packfil"; filename="ExampleApp.eap"
Content-Type: application/octet-stream
<application package data>

```

Upload a license key.

Request:

```codeBlockLines_e6Vv
POST /axis-cgi/applications/license.cgi?action=uploadlicensekey&package=ExampleApp HTTP/1.1
Content-Type: multipart/form-data; boundary=fileboundary
Content-Length: <content-length>

fileboundary
Content-Disposition: form-data; name="licenseKey"; filename="ExampleAppLicenseKey.xml"
Content-Type: application/octet-stream
<license key data>

```

Allow an unsigned application to be installed.

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/config.cgi?action=set&name=AllowUnsigned&value=true

```

Start the application.

Request:

```codeBlockLines_e6Vv
http://myserver/axis-cgi/applications/control.cgi?action=start&package=ExampleApp

```

List installed applications.

Request:

```codeBlockLines_e6Vv
http://myserver/axis-cgi/applications/list.cgi

```

Response:

```codeBlockLines_e6Vv
<reply result="ok">
    <application
        Name="CrossLineDetection"
        ApplicationID="3051"
        NiceName="AXIS Cross Line Detection"
        Vendor="Axis Communications"
        Version="1.1"
        Status="Running"
        License="Valid"
        ConfigurationPage="local/CrossLineDetection/info.html"
        VendorHomePage="https://www.axis.com"
        LicenseName="Proprietary" />
    <application
        Name="DigitalAutotracking"
        ApplicationID="6789"
        NiceName="AXIS Digital Autotracking beta1"
        Vendor="Axis Communications"
        Version="1.0"
        Status="Idle"
        License="None"
        ConfigurationPage="local/DigitalAutotracking/index.html"
        VendorHomePage="https://www.axis.com"
        LicenseName="available" />
    <application
        Name="Metadata"
        ApplicationID="22929"
        NiceName="AXIS Metadata"
        Vendor="Axis Communications"
        Version="1.0"
        Status="Stopped"
        License="Invalid"
        LicenseExpirationDate="2011-01-01"
        ConfigurationPage="local/Metadata/setup.html"
        VendorHomePage="https://www.axis.com"
        LicenseName="available" />
    <application
        Name="VideoMotionDetection"
        ApplicationID="8546"
        NiceName="AXIS Video Motion Detection"
        Vendor="Axis Communications"
        Version="2.1"
        Status="Stopped"
        License="None"
        ConfigurationPage="local/VideoMotionDetection/config.html"
        VendorHomePage="https://www.axis.com"
        LicenseName="available" />
    <application
        Name="ExampleApp"
        ApplicationID="1"
        NiceName="An Example Application"
        Vendor="Example inc"
        Version="1.0"
        Status="Stopped"
        License="Custom"
        ConfigurationPage="local/ExampleApp/settings.html"
        VendorHomePage="https://www.example.inc"
        LicenseName="Proprietary" />
</reply>

```

## Upload application [​](https://developer.axis.com/vapix/applications/application-api/\#upload-application "Direct link to Upload application")

The `applications/upload.cgi` is used to upload ACAP applications to the Axis product. Applications are installed automatically when uploaded.

### Request [​](https://developer.axis.com/vapix/applications/application-api/\#request-upload-application "Direct link to Request")

- **Access control**: admin
- **Method**: `POST`

Syntax:

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/upload.cgi

```

Body:

```codeBlockLines_e6Vv
POST http://<servername>/axis-cgi/applications/upload.cgi HTTP/1.1
Content-Type: multipart/form-data; boundary=fileboundary
Content-Length: <content-length>

fileboundary
Content-Disposition: form-data; name="packfil"; filename="application_name"
Content-Type: application/octet-stream
<application package data>

```

### Response [​](https://developer.axis.com/vapix/applications/application-api/\#response-upload-application "Direct link to Response")

Responses to `applications/upload.cgi`

**Success:**

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
OK

```

**Error:**

If the request failed, the following is returned.

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
Error: <error code>

```

| Error code | Description |
| --- | --- |
| `1` | Invalid package. The package is not an Embedded Axis Package. |
| `2` | Verification failed. The verification of the package contents failed. The signature is either missing or invalid. |
| `3` | Package too large. The package file is too large or the disk is full. |
| `5` | Package not compatible. The package is not compatible with the Axis product. See the product logs for more information. |
| `10` | Unspecified error. See the product logs for more information. |
| `12` | Upload currently unavailable. A package upload is ongoing. |
| `13` | Installation failed. The package requires a user or group that is not allowed. |
| `14` | Package already exists. The package already exists in the application store but with a different letter case. |
| `15` | Operation timed out. The outcome of the operation could not be determined. Check logs for more information. |
| `29` | Invalid manifest.json or package.conf file. |

## Control application [​](https://developer.axis.com/vapix/applications/application-api/\#control-application "Direct link to Control application")

The `applications/control.cgi` is used to start, stop, restart and remove the ACAP application.

### Request [​](https://developer.axis.com/vapix/applications/application-api/\#request-control-application "Direct link to Request")

- **Access control**: admin
- **Method**: `POST`

Syntax

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/control.cgi?<argument>=<value>
[&<argument>=<value>...]

```

With the following argument and values:

| Argument | Valid values | Description |
| --- | --- | --- |
| `action=<string>` | `start` `stop` `restart` `remove` | `start` = Start the application. `stop` = Stop the application. `restart` = Restart the application. `remove` = Remove the application. |
| `package=<string>` | `<package name>` | The application to perform the action on. |
| `returnpage=<string>` | `<path to return page>` | Optional. The web page to return to after performing the action. |

### Response [​](https://developer.axis.com/vapix/applications/application-api/\#response-control-application "Direct link to Response")

Responses to `applications/control.cgi`

**Success:**

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
OK

```

**Error:**

If the request failed, the following is returned.

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
Error: <error code>

```

| Error code | Description |
| --- | --- |
| `1` | Invalid package. The package is not an Embedded Axis Package or it contains an invalid manifest. |
| `4` | Application not found. The specified application package could not be found. |
| `6` | The application is already running. |
| `7` | Application not running. The application must be running to perform the action. |
| `9` | Too many applications are running. The application cannot be started. _This limitation was removed in AXIS OS 12.6_ |
| `10` | Unspecified error. See the product logs for more information. |
| `15` | Operation timed out. The outcome of the operation could not be determined. Check logs for more information. |

## Configure applications [​](https://developer.axis.com/vapix/applications/application-api/\#configure-applications "Direct link to Configure applications")

The `applications/config.cgi` is used to access or configure settings that are used by all ACAP applications, such as whether the device will allow unsigned applications to be installed.

**Request**

- **Access control**: admin
- **Method**: `POST`

Syntax

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/config.cgi?action=<action>&name=<configname>&value=<configvalue>

```

With the following argument and values:

| Argument | Valid values | Description |
| --- | --- | --- |
| `action=<string>` | `set` `get` | `set` = Sets the value of a config parameter. `get` = Retrieves the value of a config parameter. |
| `name=<string>` | `AllowUnsigned` (from AXIS OS 11.2), `AllowRoot` (AXIS OS 11.5-11.11) | The name of the config parameter that should be operated on. Valid values are described in the _Configuration parameters_ table below. |
| `value=<string>` _Optional_ | `true` `false` | Should be used when the action is set to specify the required value for the config parameter. |

Configuration parameters

| Configuration name | Type | Default value | Description |
| --- | --- | --- | --- |
| `AllowUnsigned` | Boolean | `true` (until AXIS OS 11.11), `false` (from AXIS OS 12.0) | Controls whether unsigned applications can be installed. Can be either `true` or `false`. |
| `AllowRoot` | Boolean | `true` (until AXIS OS 11.7), `false` (from AXIS OS 11.8) | Controls whether applications running as root can be installed, and if the install scripts are run as root or as the application user. Can be either `true` or `false`. |

**Response**

Responses to `applications/config.cgi`

**Success:**

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/xml`

Body

```codeBlockLines_e6Vv
<reply result="ok">
    <param name="configname" value="configvalue" />
</reply>

```

Response example

```codeBlockLines_e6Vv
<reply result="ok">
    <param name="AllowUnsigned" value="true" />
</reply>

```

The response body contains a `key` and `value` corresponding to the `param name="configname"` in the request.

| Parameter | Description |
| --- | --- |
| `param name=<key>` | The parameter name. |
| `value=<true | false>` | The parameter value. |

**Error:**

The following will be returned if the request failed.

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/xml`

Body

```codeBlockLines_e6Vv
<reply result="error">
    ...
</reply>

```

Response example

```codeBlockLines_e6Vv
<reply result="error">
    <error type="1" message="Error: gdbus call failed" />
</reply>

```

| Error code | Description |
| --- | --- |
| `1` | Malformed request. `value` must be either `true` or `false`. (From AXIS OS 12.2, this has been replaced with HTTP 400 Bad Request) |

### Schema for config.cgi response [​](https://developer.axis.com/vapix/applications/application-api/\#schema-for-configcgi-response "Direct link to Schema for config.cgi response")

```codeBlockLines_e6Vv
<?xml version="1.0" encoding="utf-8" ?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">
    <xs:element name="reply">
        <xs:complexType>
            <xs:sequence>
                <xs:element name="param">
                    <xs:complexType>
                        <xs:simpleContent>
                            <xs:extension base="xs:string">
                                <xs:attribute type="xs:string" name="name" />
                                <xs:attribute type="xs:string" name="value" />
                            </xs:extension>
                        </xs:simpleContent>
                    </xs:complexType>
                </xs:element>
            </xs:sequence>
            <xs:attribute type="xs:string" name="result" />
        </xs:complexType>
    </xs:element>
</xs:schema>

```

## Manage license keys [​](https://developer.axis.com/vapix/applications/application-api/\#manage-license-keys "Direct link to Manage license keys")

The `applications/license.cgi` is used to upload, install and remove license keys.

### Request [​](https://developer.axis.com/vapix/applications/application-api/\#request-manage-license-keys "Direct link to Request")

- **Access control**: admin
- **Method**: `POST`

Syntax

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/license.cgi?<argument>=<value>
[&<argument>=<value>...]

```

With the following arguments and values.

| Argument | Valid values | Description |
| --- | --- | --- |
| `action=<string>` | `uploadlicensekey` `removelicensekey` | `uploadlicensekey` = Upload and install a license key. `removelicensekey` = Remove a license key. |
| `package=<string>` | `<package name>` | The name of the application to which the license key belongs. |

### Response [​](https://developer.axis.com/vapix/applications/application-api/\#response-manage-license-keys "Direct link to Response")

Responses to `applications/license.cgi`

**Success:**

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
OK

```

**Error:**

If the request failed, the following is returned.

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/plain`

Body:

```codeBlockLines_e6Vv
Error: <error code>

```

| Error code | Description |
| --- | --- |
| `21` | Invalid license key file. |
| `22` | File upload failed. See the product logs for more information. |
| `23` | Failed to remove the license key file. See the product logs for more information. |
| `24` | The application is not installed. |
| `25` | The key’s application ID does not match the installed application. |
| `26` | The license key cannot be used with this version of the application. |
| `30` | Wrong serial number. |
| `31` | The license key has expired. |

## List installed applications [​](https://developer.axis.com/vapix/applications/application-api/\#list-installed-applications "Direct link to List installed applications")

The `applications/list.cgi` is used to list information about installed ACAP applications.

info

applications/list.cgi is supported for Properties.EmbeddedDevelopment.Version=1.20 and later.

### Request [​](https://developer.axis.com/vapix/applications/application-api/\#request-list-installed-applications "Direct link to Request")

- **Access control**: admin
- **Method**: `POST`

Syntax

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/list.cgi

```

This CGI has no arguments.

### Response [​](https://developer.axis.com/vapix/applications/application-api/\#response-list-installed-applications "Direct link to Response")

Response to `applications/list.cgi`. The response is in XML format.

For the XML schema, see [Schema for list.cgi response](https://developer.axis.com/vapix/applications/application-api/#schema-for-listcgi-response)

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/xml`

Body:

```codeBlockLines_e6Vv
<reply result="ok">
    <application
        Name="<Application Name>"
        NiceName="<Nice name>"
        Vendor="<Vendor name>"
        Version="<Version>"
        ApplicationID="<ID>"
        License="<License status>"
        LicenseExpirationDate="<Expiration date>"
        Status="<Application status>"
        ConfigurationPage="<Configuration link>"
        ValidationResult="<Validation URL>" />
        <application ... />
    ...
</reply>

```

Supported elements, attributes and values:

| Element | Description | Attribute | Description |
| --- | --- | --- | --- |
| `application` | Information about one application | `Name` | Application short name. |
|  |  | `NiceName` | Application official name. |
|  |  | `Vendor` | Application vendor. |
|  |  | `Version` | Application version. |
|  |  | `ApplicationID` | Application ID |
|  |  | `License` | License status: `Valid` = License is installed and valid. `Invalid` = License is installed but not valid. `Missing` = No license is installed. `Custom` = Custom license is used. License status cannot be retrieved. `None` = Application does not require any license. |
|  |  | `LicenseExpirationDate` | Date (YYYY-MM-DD) when the license expires. |
|  |  | `Status` | Application status: `Running` = Application is running. `Stopped` = Application is not running. `Idle` = Application is idle. |
|  |  | `ConfigurationPage` | Relative URL to application configuration page. |
|  |  | `ValidationResult` | Complete URL to a validation or result page. |

### Schema for list.cgi response [​](https://developer.axis.com/vapix/applications/application-api/\#schema-for-listcgi-response "Direct link to Schema for list.cgi response")

```codeBlockLines_e6Vv
<?xml version="1.0" encoding="utf-8" ?>
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">
    <xs:simpleType name="EnumResult">
        <xs:restriction base="xs:string">
            <xs:enumeration value="ok" />
            <xs:enumeration value="error" />
        </xs:restriction>
    </xs:simpleType>

    <xs:simpleType name="StatusType">
        <xs:restriction base="xs:string">
            <xs:enumeration value="Running" />
            <xs:enumeration value="Stopped" />
            <xs:enumeration value="Idle" />
        </xs:restriction>
    </xs:simpleType>

    <xs:simpleType name="LicenseType">
        <xs:restriction base="xs:string">
            <xs:enumeration value="Valid" />
            <xs:enumeration value="Invalid" />
            <xs:enumeration value="Missing" />
            <xs:enumeration value="Custom" />
            <xs:enumeration value="None" />
        </xs:restriction>
    </xs:simpleType>

    <xs:complexType name="ErrorType">
        <xs:attribute name="type" type="xs:string" use="required" />
        <xs:attribute name="message" type="xs:string" use="required" />
        <xs:anyAttribute processContents="lax" />
    </xs:complexType>

    <xs:complexType name="ApplicationType">
        <xs:attribute name="Name" type="xs:string" use="required" />
        <xs:attribute name="ApplicationID" type="xs:integer" />
        <xs:attribute name="NiceName" type="xs:string" />
        <xs:attribute name="Vendor" type="xs:string" />
        <xs:attribute name="Version" type="xs:string" />
        <xs:attribute name="Status" type="StatusType" />
        <xs:attribute name="License" type="LicenseType" />
        <xs:attribute name="LicenseExpirationDate" type="xs:string" />
        <xs:attribute name="ConfigurationPage" type="xs:string" />
        <xs:attribute name="VendorHomePage" type="xs:string" />
        <xs:attribute name="LicenseName" type="xs:string" />
        <xs:anyAttribute processContents="lax" />
    </xs:complexType>

    <xs:element name="reply">
        <xs:complexType>
            <xs:choice>
                <xs:element name="application" type="ApplicationType" minOccurs="0" maxOccurs="unbounded" />
                <xs:element name="error" type="ErrorType" minOccurs="0" />
            </xs:choice>
            <xs:attribute name="result" type="EnumResult" />
        </xs:complexType>
    </xs:element>
</xs:schema>

```

## Read general information [​](https://developer.axis.com/vapix/applications/application-api/\#read-general-information "Direct link to Read general information")

The `applications/info.cgi` is used to retrieve general information related to ACAP support on the device.

**Request**

- **Access control**: admin
- **Method**: `POST`

Syntax:

```codeBlockLines_e6Vv
http://<servername>/axis-cgi/applications/info.cgi

```

This CGI has no arguments.

**Response**

Responses to `applications/info.cgi`.

**Success:**

- **HTTP Code**: `200 OK`
- **Content-Type**: `text/xml`

Body:

```codeBlockLines_e6Vv
<reply result="ok">
    <supportedSdks>
        <sdk>acap3</sdk>
        <sdk>acap4-cv</sdk>
        <sdk>acap4-native</sdk>
    </supportedSdks>
</reply>

```

Supported elements, attributes and values:

| Argument | Description |
| --- | --- |
| `supportedSdks` | SDKs whose applications able to run on the device. |
| `sdk` | A specific SDK. |

Last updated on **Oct 15, 2025**

[Previous\\
\\
Applications](https://developer.axis.com/vapix/applications/) [Next\\
\\
Application configuration API](https://developer.axis.com/vapix/applications/application-configuration-api/)

- [Description](https://developer.axis.com/vapix/applications/application-api/#description)
  - [Identification](https://developer.axis.com/vapix/applications/application-api/#identification)
- [Common examples](https://developer.axis.com/vapix/applications/application-api/#common-examples)
- [Upload application](https://developer.axis.com/vapix/applications/application-api/#upload-application)
  - [Request](https://developer.axis.com/vapix/applications/application-api/#request-upload-application)
  - [Response](https://developer.axis.com/vapix/applications/application-api/#response-upload-application)
- [Control application](https://developer.axis.com/vapix/applications/application-api/#control-application)
  - [Request](https://developer.axis.com/vapix/applications/application-api/#request-control-application)
  - [Response](https://developer.axis.com/vapix/applications/application-api/#response-control-application)
- [Configure applications](https://developer.axis.com/vapix/applications/application-api/#configure-applications)
  - [Schema for config.cgi response](https://developer.axis.com/vapix/applications/application-api/#schema-for-configcgi-response)
- [Manage license keys](https://developer.axis.com/vapix/applications/application-api/#manage-license-keys)
  - [Request](https://developer.axis.com/vapix/applications/application-api/#request-manage-license-keys)
  - [Response](https://developer.axis.com/vapix/applications/application-api/#response-manage-license-keys)
- [List installed applications](https://developer.axis.com/vapix/applications/application-api/#list-installed-applications)
  - [Request](https://developer.axis.com/vapix/applications/application-api/#request-list-installed-applications)
  - [Response](https://developer.axis.com/vapix/applications/application-api/#response-list-installed-applications)
  - [Schema for list.cgi response](https://developer.axis.com/vapix/applications/application-api/#schema-for-listcgi-response)
- [Read general information](https://developer.axis.com/vapix/applications/application-api/#read-general-information)

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