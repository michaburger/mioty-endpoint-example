// MIOTY Endpoint Example Decoder for ThingsBoard Platform
// 
// This decoder is specifically designed for use on the ThingsBoard IoT platform
// to parse MIOTY protocol messages from Loriot network server integration.
//

/** Helper Functions **/

function hexStringToBytes(hex) {
    var bytes = [];
    for (var i = 0; i < hex.length; i += 2) {
        bytes.push(parseInt(hex.substr(i, 2), 16));
    }
    return bytes;
}

function tryParseJsonFromBytes(payload) {
    // Try to detect if this is JSON by attempting to parse it as a string
    if (Array.isArray(payload) && payload.length > 10 && 
        payload[0] === 123 && payload[payload.length - 1] === 125) {
        try {
            var jsonString = String.fromCharCode.apply(String, payload);
            return JSON.parse(jsonString);
        } catch (e) {
            return null;
        }
    }
    return null;
}

/** Decoder **/

function Decoder(payload, metadata) {

function getTriggerTypeName(triggerType) {
    var triggerTypes = {
        1: "TIMER",
        2: "BUTTON", 
        3: "SENSOR_THRESHOLD",
        4: "BATTERY_LOW",
        5: "ERROR_CONDITION",
        6: "MANUAL",
        7: "RFU_1",
        8: "RFU_2"
    };
    return triggerTypes[triggerType] || "UNKNOWN";
}

// Handle different payload formats more defensively
var payloadBytes;
var actualMetadata = metadata;

// Check if payload is actually JSON as byte array
if (Array.isArray(payload)) {
    var parsedJson = tryParseJsonFromBytes(payload);
    if (parsedJson && parsedJson.data && typeof parsedJson.data === 'string' && 
        parsedJson.protocol === 'mioty' && parsedJson.cmd === 'gw') {
        // Valid MIOTY JSON payload
        payloadBytes = hexStringToBytes(parsedJson.data);
        actualMetadata = parsedJson;
    } else {
        // Direct byte array payload
        payloadBytes = payload;
    }
} else if (typeof payload === 'string') {
    // Hex string payload (fallback)
    payloadBytes = hexStringToBytes(payload);
} else if (payload && payload.data && typeof payload.data === 'string') {
    // Sometimes the whole object comes as payload
    payloadBytes = hexStringToBytes(payload.data);
    actualMetadata = payload;
} else if (metadata && metadata.data && typeof metadata.data === 'string') {
    // Or the hex data is in metadata
    payloadBytes = hexStringToBytes(metadata.data);
} else {
    // Last resort: if we have a payload object, try to find the data
    var dataField = (payload && (payload.data || payload.payload)) || 
                   (metadata && (metadata.data || metadata.payload));
    if (typeof dataField === 'string') {
        payloadBytes = hexStringToBytes(dataField);
    } else if (Array.isArray(dataField)) {
        payloadBytes = dataField;
    } else {
        throw new Error("Cannot find valid payload data");
    }
}

if (payloadBytes.length < 10) {
    throw new Error("Payload too short. Expected at least 10 bytes, got " + payloadBytes.length);
}

// Extract payload header values (8 bytes header according to PayloadConfig::PayloadHeader)
var payloadVersion = payloadBytes[0];
var firmwareMajor = payloadBytes[1];
var firmwareMinor = payloadBytes[2];
var hardwareVersion = payloadBytes[3];
var txPowerDbm = payloadBytes[4]; // unsigned 8-bit
var triggerType = payloadBytes[5];
var triggerTypeName = getTriggerTypeName(triggerType);
var reserved1 = payloadBytes[6];
var reserved2 = payloadBytes[7];

// Parse sensor data starting from byte 8
// Internal temperature: int16 big endian with 100x multiplier (0.01°C precision)
var tempRaw = (payloadBytes[8] << 8) | payloadBytes[9];
// Handle signed 16-bit values (int16)
if (tempRaw > 32767) {
    tempRaw = tempRaw - 65536;
}
var temperature = tempRaw / 100.0;

// Extract gateway information (RSSI/SNR from first gateway)
var gatewayInfo = actualMetadata && actualMetadata.gws && actualMetadata.gws.length > 0 ? actualMetadata.gws[0] : {};
var rssi = gatewayInfo.rssi || null;
var snr = gatewayInfo.snr || null;

// Extract frame counter
var fcnt = (actualMetadata && actualMetadata.fcnt) || 0;

// Device identification - extract EUI properly
var deviceEUI = (actualMetadata && (actualMetadata.EUI || actualMetadata.eui)) || 'unknown';

// Only process data if EUI is known - reject unknown devices
if (deviceEUI === 'unknown') {
    throw new Error("Device EUI is unknown - data routing rejected");
}

var deviceName = 'mioty-node-' + deviceEUI.slice(-4);
var deviceType = 'mioty-example-node-temperature';
var groupName = 'Example nodes';
var manufacturer = 'mioty Alliance';

// Result object with device attributes/telemetry data
var result = {
    deviceName: deviceName,
    deviceType: deviceType,
    groupName: groupName,
    attributes: {
        payload_version: payloadVersion,
        firmware_major: firmwareMajor,
        firmware_minor: firmwareMinor,
        hardware_version: hardwareVersion,
        txpower: txPowerDbm,
        trigger_type: triggerType,
        trigger_type_name: triggerTypeName,
        reserved1: reserved1,
        reserved2: reserved2,
        device_eui: deviceEUI,
        fcnt: fcnt,
        sequence_number: actualMetadata && actualMetadata.seqno,
        rssi: rssi,
        snr: snr,
        protocol: (actualMetadata && actualMetadata.protocol) || 'mioty',
        message_type: (actualMetadata && actualMetadata.cmd) || 'rx',
        integrationName: actualMetadata && actualMetadata['integrationName'],
        manufacturer: manufacturer
    },
    telemetry: {
        temperature: temperature,
        rssi: rssi,
        snr: snr,
        fcnt: fcnt,
        ts: (actualMetadata && actualMetadata.ts) || Date.now()
    }
};

/** Helper functions **/

function decodeToString(payload) {
    return String.fromCharCode.apply(String, payload);
}

function decodeToJson(payload) {
    // convert payload to string.
    var str = decodeToString(payload);
    
    // parse string to JSON
    var data = JSON.parse(str);
    return data;
}

return result;

}

// Main execution with message filtering for ThingsBoard
// Loriot sends duplicate messages: "rx" and "gw" commands
// We only process "gw" messages to avoid duplicates and get gateway information

// Extract cmd field to filter messages
var messageCmd = null;
var parsedJson = tryParseJsonFromBytes(payload);
if (parsedJson) {
    messageCmd = parsedJson.cmd;
} else {
    // Use metadata cmd if available
    messageCmd = metadata && metadata.cmd;
}

return Decoder(payload, metadata);
