//inspired by https://github.com/T-vK/ESP32-BLE-Keyboard

#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLESecurity.h>

#include<Arduino.h>


//TODO: make private
static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop Ctrls)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  // ------------------------------------------------- Keyboard
  REPORT_ID(1),       0x01,          //   REPORT_ID (1)
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   Logical Maximum (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x01,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys
  USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
  USAGE(1),           0x01,          // USAGE (Consumer Control)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  REPORT_ID(1),       0x02,          //   REPORT_ID (3)
  USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
  USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
  USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
  USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
  USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
  USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
  USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
  USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
  USAGE(2),           0x23, 0x02,    //   Usage (WWW Home)            ; bit 7: 128
  USAGE(2),           0x94, 0x01,    //   Usage (My Computer) ; bit 0: 1
  USAGE(2),           0x92, 0x01,    //   Usage (Calculator)  ; bit 1: 2
  USAGE(2),           0x2A, 0x02,    //   Usage (WWW fav)     ; bit 2: 4
  USAGE(2),           0x21, 0x02,    //   Usage (WWW search)  ; bit 3: 8
  USAGE(2),           0x26, 0x02,    //   Usage (WWW stop)    ; bit 4: 16
  USAGE(2),           0x24, 0x02,    //   Usage (WWW back)    ; bit 5: 32
  USAGE(2),           0x83, 0x01,    //   Usage (Media sel)   ; bit 6: 64
  USAGE(2),           0x8A, 0x01,    //   Usage (Mail)        ; bit 7: 128
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0)                  // END_COLLECTION
};

//TODO: make configurable
#define SHIFT 0x80
const uint8_t _asciimap[128] =
{
  
  0x37 | SHIFT,    // >
  0x38 | SHIFT,    // ?
  0x1f | SHIFT,    // @
  0x04 | SHIFT,    // A
  0x05 | SHIFT,    // B
  0x06 | SHIFT,    // C
  0x07 | SHIFT,    // D
  0x08 | SHIFT,    // E
  0x09 | SHIFT,    // F
  0x0a | SHIFT,    // G
  0x0b | SHIFT,    // H
  0x0c | SHIFT,    // I
  0x0d | SHIFT,    // J
  0x0e | SHIFT,    // K
  0x0f | SHIFT,    // L
  0x10 | SHIFT,    // M
  0x11 | SHIFT,    // N
  0x12 | SHIFT,    // O
  0x13 | SHIFT,    // P
  0x14 | SHIFT,    // Q
  0x15 | SHIFT,    // R
  0x16 | SHIFT,    // S
  0x17 | SHIFT,    // T
  0x18 | SHIFT,    // U
  0x19 | SHIFT,    // V
  0x1a | SHIFT,    // W
  0x1b | SHIFT,    // X
  0x1c | SHIFT,    // Y
  0x1d | SHIFT,    // Z
  0x2f,          // [
  0x31,          // bslash
  0x30,          // ]
  0x23 | SHIFT,  // ^
  0x2d | SHIFT,  // _
  0x35,          // `
  0x04,          // a
  0x05,          // b
  0x06,          // c
  0x07,          // d
  0x08,          // e
  0x09,          // f
  0x0a,          // g
  0x0b,          // h
  0x0c,          // i
  0x0d,          // j
  0x0e,          // k
  0x0f,          // l
  0x10,          // m
  0x11,          // n
  0x12,          // o
  0x13,          // p
  0x14,          // q
  0x15,          // r
  0x16,          // s
  0x17,          // t
  0x18,          // u
  0x19,          // v
  0x1a,          // w
  0x1b,          // x
  0x1c,          // y
  0x1d,          // z
  0x2f | SHIFT,  // {
  0x31 | SHIFT,  // |
  0x30 | SHIFT,  // }
  0x35 | SHIFT,  // ~
  0       // DEL
};

class BLEKeyboardCallbacks {
  public:
    virtual void authenticationInfo(uint32_t pin) = 0;
};

class BLEKeyboard : BLEServerCallbacks,  BLESecurityCallbacks {
  private:
    typedef struct
    {
      uint8_t modifiers;
      uint8_t reserved;
      uint8_t keys[6];
    } KeyReport;

    std::string deviceName;
    std::string deviceManufacturer;
    BLEHIDDevice* hid;
    BLEKeyboardCallbacks* callbacks;
    bool connected = false;
    BLECharacteristic* inputKeyboard;
    KeyReport _keyReport;

  public:
    BLEKeyboard(std::string deviceName = "ESPWV32", std::string deviceManufacturer = "Espressif") {
      this->deviceName = deviceName;
      this->deviceManufacturer = deviceManufacturer;

      BLEDevice::init(this->deviceName);
      BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
      BLEServer *pServer = BLEDevice::createServer();
      pServer->setCallbacks(this);
      BLEDevice::setSecurityCallbacks(this);



      BLESecurity *pSecurity = new BLESecurity();
      pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
      pSecurity->setCapability(ESP_IO_CAP_OUT);
      pSecurity->setKeySize(16);
      pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
      pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

      this->hid = new BLEHIDDevice(pServer);
      hid->manufacturer()->setValue(this->deviceManufacturer);
      this->inputKeyboard = hid->inputReport(0x01);
      hid->outputReport(0x01);
      hid->inputReport(0x02);
      hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
      hid->hidInfo(0x00, 0x01);
      hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
      hid->startServices();


      BLEAdvertising *pAdvertising = pServer->getAdvertising();
      pAdvertising->setScanResponse(true);
      pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
      pAdvertising->setMinPreferred(0x12);
      pAdvertising->setAppearance(HID_KEYBOARD);
      pAdvertising->addServiceUUID(hid->hidService()->getUUID());
      pAdvertising->start();
      Serial.println("Advertising started!");
    };

    ~BLEKeyboard() {
      //TODO: disable Bluetooth
      delete &deviceName;
      delete &deviceManufacturer;
    }

    void onConnect(BLEServer* pServer)
    {
      //TODO: Notify of incomming connection
    }

    void onDisconnect(BLEServer* pServer)
    {
      this->connected = false;
      //TODO: Notify connection lost
    }

    uint32_t onPassKeyRequest()
    {
      //TODO: Error + disconenct
      return UINT32_MAX;
    }

    void onPassKeyNotify(uint32_t pass_key)
    {
      //TODO notify we need to display the PIN
      if (callbacks != NULL) {
        callbacks->authenticationInfo(pass_key);
      } else {
        Serial.println("No callback to notify");
      }
    }

    bool onSecurityRequest()
    {

      return true;
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
    {
      if (cmpl.success)
      {
        //TODO: Add device to the whitelist: esp_ble_gap_update_whitelist
        //TODO: Notify connection succeeded
        this->connected = true;
      } else {

        //TODD: disconnect pServer->disconnect(pServer->getConnId());
        //TODO: Notify connection succeeded
      }
    }

    bool onConfirmPIN(unsigned int v)
    {
      //TODO: Error + disconenct
      return true;
    }

    bool isConnected() {
      return this->connected;
    }

    void sendReport(KeyReport* keys)
    {
      if (this->isConnected())
      {
        this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
        this->inputKeyboard->notify();
      }
    }

    size_t press(uint8_t k)
    {
      _keyReport.keys[0] = _asciimap[k];
      sendReport(&_keyReport);
      return 1;
    }

    size_t release(uint8_t k)
    {
      uint8_t i;
      if (k >= 136) {     // it's a non-printing key (not a modifier)
        k = k - 136;
      } else if (k >= 128) {  // it's a modifier key
        _keyReport.modifiers &= ~(1 << (k - 128));
        k = 0;
      } else {        // it's a printing key
        k = _asciimap[k];
        if (!k) {
          return 0;
        }
        if (k & 0x80) {             // it's a capital letter or other character reached with shift
          _keyReport.modifiers &= ~(0x02);  // the left shift modifier
          k &= 0x7F;
        }
      }

      // Test the key report to see if k is present.  Clear it if it exists.
      // Check all positions in case the key is present more than once (which it shouldn't be)
      for (i = 0; i < 6; i++) {
        if (0 != k && _keyReport.keys[i] == k) {
          _keyReport.keys[i] = 0x00;
        }
      }

      sendReport(&_keyReport);
      return 1;
    }

    size_t write(uint8_t c)
    {
      uint8_t p = press(c);  // Keydown
      release(c);            // Keyup
      return p;              // just return the result of press() since release() almost always returns 1
    }

    size_t write(const uint8_t *buffer, size_t size) {
      size_t n = 0;
      while (size--) {
        if (*buffer != '\r') {
          if (write(*buffer)) {
            n++;
          } else {
            break;
          }
        }
        buffer++;
      }
      return n;
    }

    void sendText(std::string text) {

    }

    void setBatteryLevel(uint8_t level) {
      this->hid->setBatteryLevel(level);
    }

    void setCallbacks(BLEKeyboardCallbacks* callbacks) {
      this->callbacks=callbacks;
    }


};