/*
Copyright (c) 2014 Paul Gavrikov
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

import Cocoa
import CoreBluetooth

class ViewController: NSViewController, CBCentralManagerDelegate, CBPeripheralDelegate {
    
    var centralManager:CBCentralManager!
    var discovered = false
    var connectingPeripheral: CBPeripheral!
    var batteryCharacteristic: CBCharacteristic!
    
    var  flagSent: UInt8=0
    
    @IBOutlet weak var stepsView: NSTextField!
    @IBOutlet weak var spinnerView: NSProgressIndicator!
    @IBOutlet weak var batteryView: NSTextField!
    @IBOutlet weak var resistanceView: NSTextField!
    
    @IBOutlet weak var batteryLevel: NSButtonCell!
    
    @IBOutlet weak var history: NSButtonCell!
    
    @IBAction func refreshButton(sender: AnyObject) {
        discoverDevices()
    }
    
    

    @IBAction func batteryButton (sender: NSButtonCell!)
    {
        batteryView.stringValue = "fetching battery voltage"
        
        //_selectedPeripheral?.writeValue(data, forCharacteristic: characteristic!, type: CBCharacteristicWriteType.WithResponse)
        
        if batteryCharacteristic == nil {
            return
        }else{
            
            sendFlag(3)

        }
        
    }
    
    
    
    @IBAction func historyButton (sender: NSButtonCell!)
    {
        batteryView.stringValue = "fetching history"
        
        //_selectedPeripheral?.writeValue(data, forCharacteristic: characteristic!, type: CBCharacteristicWriteType.WithResponse)
        
        if batteryCharacteristic == nil {
            return
        }else{
            
            sendFlag(2)
            
        }
        
    }
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        spinnerView.startAnimation(nil)
        startUpCentralManager()
        
    }
    
    func sendFlag(flag: UInt8){
        /*
        switch flag {
        case 0:
            return
        case 1:
            return
        case 2:
            return
        case 3:
            return
        default:
            return
        }*/
        print("writing to ")
        println(batteryCharacteristic.UUID.UUIDString)
        println(batteryCharacteristic)
        var bytes:[UInt8] = [flag];
        var toSend:NSData = NSData(bytes: bytes, length: bytes.count);
        
        
        connectingPeripheral.writeValue(toSend, forCharacteristic: batteryCharacteristic, type: CBCharacteristicWriteType.WithoutResponse)
        flagSent = flag
        
    }
    
    
    func startUpCentralManager() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func discoverDevices() {
        
        if(connectingPeripheral != nil) {
            centralManager.cancelPeripheralConnection(connectingPeripheral)
        }
        
        stepsView.stringValue = "Searching"
        batteryView.stringValue = ""
        spinnerView.hidden = false
        println("discovery")
        centralManager.scanForPeripheralsWithServices(nil, options: nil)
    }
    // CBCentralManagerDelegate - This is called with the CBPeripheral class as its main input parameter. This contains most of the information there is to know about a BLE peripheral.
    
    //WAS edits here
    func centralManager(central: CBCentralManager!,
        didDiscoverPeripheral peripheral: CBPeripheral!,
        advertisementData: [NSObject : AnyObject]!,
        RSSI: NSNumber!){
            
            
            // func centralManager(central: CBCentralManager!, didDiscoverPeripheral peripheral: CBPeripheral!, advertisementData: (NSDictionary), RSSI: NSNumber!) {
            
            if (peripheral.name != nil) {
                println("Discovered: " + peripheral.name)
                println("ID: " + peripheral.identifier.UUIDString)
                
                
                
                /*
                if(peripheral.name == "MI" && (peripheral.identifier.UUIDString == "8280560D-4126-4B74-A015-DF6904878071")) {
                stepsView.stringValue = "Connecting to MI"
                self.connectingPeripheral = peripheral
                centralManager.stopScan()
                self.centralManager.connectPeripheral(peripheral, options: nil)
                } else {
                println("skipped " + peripheral.name )
                }*/
                
                
                //here
                
                
                
                //1st proto if(peripheral.name == "RFduino" && (peripheral.identifier.UUIDString == "D65B3DB4-B47B-4C7F-87D9-5459442595E9")) {
                //breadboard proto2 if(peripheral.name == "RFduino" && (peripheral.identifier.UUIDString == "4A67CF27-F64C-4D5E-8FE4-97902651D3AB")) {
                if(peripheral.name == "Snapino" ) {

                    stepsView.stringValue = "Connecting to RFDuino"
                    self.connectingPeripheral = peripheral
                    centralManager.stopScan()
                    self.centralManager.connectPeripheral(peripheral, options: nil)
                } else {
                    println("skipped " + peripheral.name )
                }
                
                
            }
            
            
    }
    // method called whenever the device state changes.
    
    func centralManagerDidUpdateState(central: CBCentralManager!) { //BLE status
        var msg = ""
        switch (central.state) {
        case .PoweredOff:
            msg = "CoreBluetooth BLE hardware is powered off"
            println("\(msg)")
            stepsView.stringValue = "Please turn on Bluetooth and retry"
            
        case .PoweredOn:
            msg = "CoreBluetooth BLE hardware is powered on and ready"
            if(!discovered) {
                discovered = true
                discoverDevices()
            }
            
        case .Resetting:
            var msg = "CoreBluetooth BLE hardware is resetting"
            
        case .Unauthorized:
            var msg = "CoreBluetooth BLE state is unauthorized"
            
        case .Unknown:
            var msg = "CoreBluetooth BLE state is unknown"
            
        case .Unsupported:
            var msg = "CoreBluetooth BLE hardware is unsupported on this platform"
            stepsView.stringValue = "Your Mac does not support BLE"
            
        }
        output("State", data: msg)
    }
    // method called whenever you have successfully connected to the BLE peripheral
    
    func centralManager(central: CBCentralManager!,didConnectPeripheral peripheral: CBPeripheral!)
    {
        peripheral.delegate = self
        peripheral.discoverServices(nil)
        
    }
    // CBPeripheralDelegate - Invoked when you discover the peripheral's available services.
    
    func peripheral(peripheral: CBPeripheral!, didDiscoverServices error: NSError!)
    {
        println("peripherial services")
        if let servicePeripherals = peripheral.services as? [CBService]
        {
            for servicePeripheral in servicePeripherals
            {
                peripheral.discoverCharacteristics(nil, forService: servicePeripheral)
                
            }
            
        }
    }
    
    func refreshBLE() {
        centralManager.scanForPeripheralsWithServices(nil, options: nil)
    }
    
    // Invoked when you discover the characteristics of a specified service.
    
    func peripheral(peripheral: CBPeripheral!, didDiscoverCharacteristicsForService service: CBService!, error: NSError!) {
        println("hello")
        stepsView.stringValue = ("connected to strech sensor")
        
        if let charactericsArr = service.characteristics  as? [CBCharacteristic]
        {
            for cc in charactericsArr
            {
                //subscribe to all notifications
                peripheral.setNotifyValue(true, forCharacteristic: cc)
                
                
                if cc.UUID.UUIDString == "2222"{
                    
                    batteryCharacteristic = cc;
               
                }
                
                /*
                if cc.UUID.UUIDString == "FF0F"{
                // pair with miband
                output("Characteristic", data: cc)
                let data: NSData = "2".dataUsingEncoding(NSUTF8StringEncoding)!
                peripheral.writeValue(data, forCharacteristic: cc, type: CBCharacteristicWriteType.WithoutResponse)
                output("Characteristic", data: cc)
                
                } else if cc.UUID.UUIDString == "FF06" {
                println("READING STEPS")
                peripheral.readValueForCharacteristic(cc)
                } else if cc.UUID.UUIDString == "FF0C" {
                println("READING BATTERY")
                peripheral.readValueForCharacteristic(cc)
                }
                
                
                //was
                else if cc.UUID.UUIDString == "FF07" {
                println("activity data")
                peripheral.readValueForCharacteristic(cc)
                }*/
                
                
                
            }
            
        }
    }
    // Invoked when you retrieve a specified characteristic's value, or when the peripheral device notifies your app that the characteristic's value has changed.
    func peripheral(peripheral: CBPeripheral!, didUpdateValueForCharacteristic characteristic: CBCharacteristic!, error: NSError!) {
        
        //output("Data for "+characteristic.UUID.UUIDString, data: characteristic.value())
        
        if(characteristic.UUID.UUIDString == "FF06") {
            spinnerView.hidden = true
            var u16 = UnsafePointer<Int>(characteristic.value().bytes).memory
            stepsView.stringValue = ("\(u16) steps")
        } else if(characteristic.UUID.UUIDString == "FF0C") {
            spinnerView.hidden = true
            var u16 = UnsafePointer<Int32>(characteristic.value().bytes).memory
            u16 =  u16 & 0xff
            batteryView.stringValue = ("\(u16) Resistance")
        }
            
            
            //was
        else if(characteristic.UUID.UUIDString == "FF07") {
            //spinnerView.hidden = true
            //println (characteristic.value().bytes)
            
            
            
            
            let theString:NSString = NSString(data: characteristic.value(), encoding: NSASCIIStringEncoding)!
            println(theString)
            
            
            
            // var u16 = UnsafePointer<Int32>(characteristic.value().bytes).memory
            
            //u16 =  u16 & 0xff
            //batteryView.stringValue = ("\(u16) % charged")
        }
        else if(characteristic.UUID.UUIDString == "2221") {
            spinnerView.hidden = true
            
            if (flagSent == 3)
            {
                print("battery voltage: ")
                
                if let str : NSString = NSString(data: characteristic.value(), encoding: NSUTF8StringEncoding) {
                    println(str)
                    batteryView.stringValue = str as String
                } else {
                    println("not a valid UTF-8 sequence")
                }
                
                flagSent=0
                
            }
                
            if(flagSent == 2){
                
                print(" history: ")
                
                if let str : NSString = NSString(data: characteristic.value(), encoding: NSUTF8StringEncoding) {
                    println(str)
                    batteryView.stringValue = "history received"
                } else {
                    println("not a valid UTF-8 sequence") 
                }
                
                //flagSent=0
            }
            
            
            
            
            
            if (false) {
            
            var u16 = UnsafePointer<Int32>(characteristic.value().bytes).memory
            //u16 =  u16 & 0xff
            resistanceView.stringValue = ("\(u16) Resistance")
            
            
            let file = "resistance.txt"
            
            if let dirs : [String] = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.DocumentDirectory, NSSearchPathDomainMask.AllDomainsMask, true) as? [String] {
                let dir = dirs[0] //documents directory
                let path = dir.stringByAppendingPathComponent(file);
                let text = "\(u16)";
                
                //writing
                text.writeToFile(path, atomically: false, encoding: NSUTF8StringEncoding, error: nil);
                
                //reading
                //let text2 = String(contentsOfFile: path, encoding: NSUTF8StringEncoding, error: nil)
                }
            }
            
            
        }
        
        
    }
    
    func output(description: String, data: AnyObject){
        println("\(description): \(data)")
        // textField.text = textField.text + "\(description): \(data)\n"
    }
    
    
    
    
    override var representedObject: AnyObject? {
        didSet {
        }
    }
}

