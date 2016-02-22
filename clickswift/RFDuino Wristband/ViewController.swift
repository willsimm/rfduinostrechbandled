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
    
    var historyStart:Bool=false;
    var historyTime:Int=0;
    
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
        print("writing to ", terminator: "")
        print(batteryCharacteristic.UUID.UUIDString)
        print(batteryCharacteristic)
        let bytes:[UInt8] = [flag];
        let toSend:NSData = NSData(bytes: bytes, length: bytes.count);
        
        
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
        print("discovery")
        centralManager.scanForPeripheralsWithServices(nil, options: nil)
    }
    // CBCentralManagerDelegate - This is called with the CBPeripheral class as its main input parameter. This contains most of the information there is to know about a BLE peripheral.
    
    //WAS edits here
    func centralManager(central: CBCentralManager,
        didDiscoverPeripheral peripheral: CBPeripheral,
        advertisementData: [String : AnyObject],
        RSSI: NSNumber){
            
            
            // func centralManager(central: CBCentralManager!, didDiscoverPeripheral peripheral: CBPeripheral!, advertisementData: (NSDictionary), RSSI: NSNumber!) {
            
            if (peripheral.name != nil) {
                print("Discovered: " + peripheral.name!)
                print("ID: " + peripheral.identifier.UUIDString)
                
                
                
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
                if(peripheral.name == "SnapClicker" ) {

                    stepsView.stringValue = "Connecting to RFDuino"
                    print("connecting" )
                    self.connectingPeripheral = peripheral
                    print("connecting b" )
                    centralManager.stopScan()
                    print("connecting c" )
                    self.centralManager.connectPeripheral(peripheral, options: nil)
                    print("connecting d" )
                } else {
                    print("skipped " + peripheral.name! )
                }
                
                
            }
            
            
    }
    // method called whenever the device state changes.
    
    func centralManagerDidUpdateState(central: CBCentralManager) { //BLE status
        var msg = ""
        switch (central.state) {
        case .PoweredOff:
            msg = "CoreBluetooth BLE hardware is powered off"
            print("\(msg)")
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
    
    func centralManager(central: CBCentralManager,didConnectPeripheral peripheral: CBPeripheral)
    {
        peripheral.delegate = self
        peripheral.discoverServices(nil)
        
    }
    // CBPeripheralDelegate - Invoked when you discover the peripheral's available services.
   /*
    func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
    {
        print("peripherial services")
        if let servicePeripherals = peripheral.services! as [CBService]
        {
            for servicePeripheral in servicePeripherals
            {
                peripheral.discoverCharacteristics(nil, forService: servicePeripheral)
                
            }
            
        }
    }*/
    let BLEServiceUUID = CBUUID(string: "025A7775-49AA-42BD-BBDB-E2AE77782966")

//    let PositionCharUUID = CBUUID(string: "F38A2C23-BC54-40FC-BED0-60EDDA139F47")
    let PositionCharUUID = CBUUID(string: "8BDF9A60-1680-4143-A804-0CD537C9F627")

    
    func peripheral(peripheral: CBPeripheral!, didDiscoverServices error: NSError!) {
        print("erg")
        let uuidsForBTService: [CBUUID] = [PositionCharUUID]
        

        
        if (error != nil) {
            return
        }
        
        if ((peripheral.services == nil) || (peripheral.services!.count == 0)) {
            print("no services")
            // No Services
            return
        }
        
        for service in peripheral.services! {
            print("serivice" + service.description)
            
            peripheral.discoverCharacteristics(nil, forService: service as CBService)
            
           // if service.UUID == BLEServiceUUID {
            //    print("match")
                //peripheral.discoverCharacteristics(uuidsForBTService, forService: service as CBService)
            //    peripheral.discoverCharacteristics(uuidsForBTService, forService: peripheral.services! as [CBService])
                
            //}
        }
    }
    
    
    
    
    func refreshBLE() {
        centralManager.scanForPeripheralsWithServices(nil, options: nil)
    }
    
    // Invoked when you discover the characteristics of a specified service.
    
    func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?) {
        print("hello")
        stepsView.stringValue = ("connected to strech sensor")
        
       print( service.characteristics?.description)
        
        for c in (service.characteristics as [CBCharacteristic]!) {
            print("hello a")
        }

        
    //    for (CBCharacteristic, *characteristic in service.characteristics) {
      //      NSLog("Discovered characteristic %@", characteristic);
            
        //}
        

        
        
        
        
        
        for characteristic in service.characteristics! {
            print("hello 2")
            
            
            //subscribe to all notifications
            peripheral.setNotifyValue(true, forCharacteristic: characteristic as CBCharacteristic)
            
            
            if characteristic.UUID.UUIDString == "2222"{
                
                batteryCharacteristic = characteristic;
                
            }
            
        }
        
        
        /*
        
        
        if let charactericsArr = service.characteristics!  as [CBCharacteristic]
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
            
        }*/
    }
    // Invoked when you retrieve a specified characteristic's value, or when the peripheral device notifies your app that the characteristic's value has changed.
    func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?) {
        
        //output("Data for "+characteristic.UUID.UUIDString, data: characteristic.value())
        
        if(characteristic.UUID.UUIDString == "FF06") {
            spinnerView.hidden = true
            var u16 = UnsafePointer<Int>(characteristic.value!.bytes).memory
            stepsView.stringValue = ("\(u16) steps")
        } else if(characteristic.UUID.UUIDString == "FF0C") {
            spinnerView.hidden = true
            var u16 = UnsafePointer<Int32>(characteristic.value!.bytes).memory
            u16 =  u16 & 0xff
            batteryView.stringValue = ("\(u16) Resistance")
        }
            
            
            //was
        else if(characteristic.UUID.UUIDString == "FF07") {
            //spinnerView.hidden = true
            //println (characteristic.value().bytes)
            
            
            
            
            let theString:NSString = NSString(data: characteristic.value!, encoding: NSASCIIStringEncoding)!
            print(theString)
            
            
            
            // var u16 = UnsafePointer<Int32>(characteristic.value().bytes).memory
            
            //u16 =  u16 & 0xff
            //batteryView.stringValue = ("\(u16) % charged")
        }
        else if(characteristic.UUID.UUIDString == "2221") {
            spinnerView.hidden = true
            
            if (flagSent == 3)
            {
                print("battery voltage: ", terminator: "")
                
                if let str : NSString = NSString(data: characteristic.value!, encoding: NSUTF8StringEncoding) {
                    print(str)
                    batteryView.stringValue = str as String
                } else {
                    print("not a valid UTF-8 sequence")
                }
                
                flagSent=0
                
            }
                
            if(flagSent == 2){
                
                print(" history: ", terminator: "")
                
                if let str : NSString = NSString(data: characteristic.value!, encoding: NSUTF8StringEncoding) {
                    print(str)
                    batteryView.stringValue = "history received"
                    logHistory(str as String);
                } else {
                    print("not a valid UTF-8 sequence")
                }
                
                //flagSent=0
            }
            
            
            
            
            
            if (false) {
            
            var u16 = UnsafePointer<Int32>(characteristic.value!.bytes).memory
            //u16 =  u16 & 0xff
            resistanceView.stringValue = ("\(u16) Resistance")
            
            
            let file = "resistance.txt"
            
            if let dirs : [String] = NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory.DocumentDirectory, NSSearchPathDomainMask.AllDomainsMask, true) as? [String] {
                let dir = dirs[0] //documents directory
                let path = (dir as NSString).stringByAppendingPathComponent(file);
                let text = "\(u16)";
                
                do {
                    //writing
                    try text.writeToFile(path, atomically: false, encoding: NSUTF8StringEncoding)
                } catch _ {
                };
                
                //reading
                //let text2 = String(contentsOfFile: path, encoding: NSUTF8StringEncoding, error: nil)
                }
            }
            
            
        }
        
        
    }
    
    //break out the history
    //this orignially dealt with pairs of values for the stretch band, now just treats them individually
    func logHistory(historyStr: String){
        let historyArray = historyStr.characters.split{$0 == "/"}.map { String($0) }
        
        //historyArray[0] // start
        //historyArray[1] // stop
        
        
        // toInt returns optional that's why we used a:Int?
        let start:Int? = Int(historyArray[0]) // firstText is UITextField
        let stop:Int? = Int(historyArray[1]) // secondText is UITextField
        
        // check a and b before unwrapping using !
        if ((start != nil)&&(stop != nil)&&(stop > start)) {
            
            if ((start == -1) && (!historyStart)){
                historyStart = true;
                print ("start of history")
                //ERASE CSV
                let date = NSDate().timeIntervalSince1970
                let currentTime:Int = Int(date)
                historyTime =  (currentTime * 1000) - stop!
                print("current time", terminator: "")
                print(currentTime)
                print("historyTime", terminator: "")
                print(historyTime)
            }
            
            else if((start == -2) && (historyStart)){
                historyStart = false;
                print ("end of history")
            }
            
            else {
                /*
                this from wrtiting each value and zeroing rather than start stop times and length
                
                //do something
                let length:Int = 0
                let startTime:Int = historyTime + start!
                //write to csv
                appendToCSV(startTime,length: length );
                
                let secondTime:Int = historyTime + stop!
                appendToCSV(secondTime,length: length);*/
                
                let length:Int = stop! - start!
                let startTime:Int = historyTime + start!
                //write to csv
                appendToCSV(startTime,length: length );
                
                
                
                
                
                

            }
            
            
        } else {
            print("Input values are not numeric")
        }
        
    }
    
    func appendToCSV(startTime: Int, length: Int){
        
        print(startTime, terminator: "")
        print (",", terminator: "")
        print(length)
        
        
        
        let dir:NSURL = NSFileManager.defaultManager().URLsForDirectory(NSSearchPathDirectory.CachesDirectory, inDomains: NSSearchPathDomainMask.UserDomainMask).last! as NSURL
        let fileurl =  dir.URLByAppendingPathComponent("log.txt")
        print(fileurl)
        
        
        let string = String(startTime) + "," + String(length) + "\n"
        let data = string.dataUsingEncoding(NSUTF8StringEncoding, allowLossyConversion: false)!
        
        if NSFileManager.defaultManager().fileExistsAtPath(fileurl.path!) {
            var err:NSError?
            do {
                let fileHandle = try NSFileHandle(forWritingToURL: fileurl)
                fileHandle.seekToEndOfFile()
                fileHandle.writeData(data)

                
                
                fileHandle.closeFile()
            } catch let error as NSError {
                err = error
                print("Can't open fileHandle \(err)")
            }
        }
        else {
            var err:NSError?
            do {
                try data.writeToURL(fileurl, options: .DataWritingAtomic)
            } catch let error as NSError {
                err = error
                print("Can't write \(err)")
            }
        }
        
        
    }
    
    
    func output(description: String, data: AnyObject){
        print("\(description): \(data)")
        // textField.text = textField.text + "\(description): \(data)\n"
    }
    
    
    
    
    override var representedObject: AnyObject? {
        didSet {
        }
    }
}
