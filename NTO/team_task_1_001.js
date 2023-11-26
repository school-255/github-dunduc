'use strict';

var wheels_bus;
var gyros;
var nav;
var transmitter;
var receiver;

let Ts = 0.01;
let I = 0;
let Kp = 0.0005, Ki = 0;
let max_momentum = 0.0001;
let out;

function pi(let input){
    let a = Kp * input;
    let P = a;
    out = P + I;
    
    if(out == constrain(out, -1 * max_momentum, max_momentum)){
        I += a * Ts * Ki;
    } 
    
    out = constrain(out, -1 * max_momentum, max_momentum);
    return out;
}

function packet_check(let raw_packet){
    if(raw_packet.length >= 14){
        let checked_packet = raw_packet;
        let crc = crc16(checked_packet, 1, checked_packet.length - 4);
        let pack_crc = checked_packet.at(-3) * 256 + checked_packet.at(-2);        
        if(crc == pack_crc){
            checked_packet[2] = 2;
            return checked_packet;
        }
    }
    return undefined;
}

function crc16(data, offset, length) {
    if (data == null || offset < 0 || offset > data.length - 1 || offset + length > data.length) { 
        return 0;
    }
    crc = 0xFFFF;
    for (i = 0; i < length; ++i) {
       crc ^= data[offset + i] << 8; 
       for (j = 0; j < 8; ++j) {
            crc = (crc & 0x8000) > 0 ? (crc << 1) ^ 0x1021 : crc << 1;
       }
    } 
    return crc & 0xFFFF;
}

function constrain(num, min, max){
    const MIN = min ?? 1;
    const MAX = max ?? 20;
    const parsed = parseInt(num);
    return Math.min(Math.max(parsed, MIN), MAX);
}

function setup() {
    transmitter = spacecraft.devices[0].functions[0];
    receiver = spacecraft.devices[1].functions[0];
    gyros = spacecraft.devices[2];
    nav = spacecraft.devices[3];
    wheels_bus = spacecraft.devices[4].functions[0];    
    wheels_bus.disable();
}
 
function loop() {
    let drive = pi(gyros.functions[0].angular_velocity);

    wheels_bus.enable();
    var byteView = new Uint8Array(drive);
    wheels_bus.transmit(byteView);


    let packet = receiver.receive(104);
    
    let normalized_packet = check_packet(packet);
    
    if(normalized_packet != undefined){
        transmitter.transmit(normalized_packet);
    }
}