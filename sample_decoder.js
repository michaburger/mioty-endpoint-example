// Decode an uplink message from a buffer
// payload - array of bytes
// metadata - key/value object

/** Decoder **/

function hexStringToBytes(hex) {
    var bytes = [];
    for (var i = 0; i < hex.length; i += 2) {
        bytes.push(parseInt(hex.substr(i, 2), 16));
    }
    return bytes;
}

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

// Handle payload format - convert hex string to bytes if needed
var payloadBytes;
if (typeof payload === 'string') {
    // Hex string payload
    payloadBytes = hexStringToBytes(payload);
} else if (Array.isArray(payload)) {
    // Byte array payload
    payloadBytes = payload;
} else {
    throw new Error("Unsupported payload format");
}

if (payloadBytes.length < 10) {
    throw new Error("Payload too short. Expected 10 bytes, got " + payloadBytes.length);
}

// Extract payload header values
var payloadVersion = payloadBytes[0];
var firmwareMajor = payloadBytes[1];
var firmwareMinor = payloadBytes[2];
var hardwareVersion = payloadBytes[3];
var txPowerDbm = (payloadBytes[4] << 24) >> 24; // signed 8-bit
var triggerType = payloadBytes[5];
var triggerTypeName = getTriggerTypeName(triggerType);
var reserved1 = payloadBytes[6];
var reserved2 = payloadBytes[7];

// Parse temperature: 16-bit signed, little endian, divide by 100
var tempRaw = payloadBytes[8] | (payloadBytes[9] << 8);
// Handle signed 16-bit values
if (tempRaw > 32767) {
    tempRaw = tempRaw - 65536;
}
var temperature = tempRaw / 100.0;

// Extract gateway information (RSSI/SNR from first gateway or top level)
var gatewayInfo = metadata.gws && metadata.gws.length > 0 ? metadata.gws[0] : {};
var rssi = metadata.rssi || gatewayInfo.rssi || null;
var snr = metadata.snr || gatewayInfo.snr || null;

// Extract frame counter - check multiple possible locations including Cyrillic 'С'
var fcnt = metadata.fcnt || metadata.fCnt || metadata['fСnt'] || metadata.frameCounter || metadata.frame_counter || 0;

// Device identification
var deviceEUI = metadata.EUI || metadata.eui || 'unknown';
var deviceName = 'mioty-node-' + (deviceEUI !== 'unknown' ? deviceEUI.slice(-4) : 'unknown');
var deviceType = 'mioty-temperature-sensor';
var customerName = 'MyCustomer';
var groupName = 'Temperature Nodes';
var manufacturer = 'mioty Alliance';

// Result object with device attributes/telemetry data
var result = {
    deviceName: deviceName,
    deviceType: deviceType,
    customerName: customerName,
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
        sequence_number: metadata.seqno,
        rssi: rssi,
        snr: snr,
        protocol: metadata.protocol || 'mioty',
        message_type: metadata.cmd || 'rx',
        integrationName: metadata['integrationName'],
        manufacturer: manufacturer
    },
    telemetry: {
        temperature: temperature,
        rssi: rssi,
        snr: snr,
        fcnt: fcnt,
        ts: metadata.ts || Date.now()
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
