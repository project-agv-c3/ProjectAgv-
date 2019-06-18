//Import all the necessairy libraries
import android.content.Intent;
import android.os.Bundle;
import ketai.net.bluetooth.*;

//Define the bluetooth object named "bt"
KetaiBluetooth bt;

void onCreate(Bundle savedInstanceState) {
  super.onCreate(savedInstanceState);
  bt = new KetaiBluetooth(this);
}
void onActivityResult(int requestCode, int resultCode, Intent data) {
  bt.onActivityResult(requestCode, resultCode, data);
}

PGraphics batteryLogo;
PGraphics treeLogo;
PGraphics modeLogo;

int state = 0;
int selected = 0;

String[] names = new String[0];
color backColor = color(255, 210, 150);
color strokeColor = color(50);
color fillColor = color(245, 152, 2);
color overColor = color(50, 80);
PVector lastMouse = new PVector(0, 0);
String deviceName;
String sMode = "Automatic";
String sTrees = "0";
String sBattery = "12,0";

void setup() {
  fullScreen();
  orientation(PORTRAIT);
  textSize(70);
  textAlign(CENTER, CENTER);
  bt.start();
  ArrayList<String> _names;
  _names = bt.getPairedDeviceNames();
  for (int i = 0; i < _names.size(); i++) {
    names = append(names, _names.get(i));
  }
  state = 1;
}

void draw() {
  switch (state) {
    case 1:  //Select your AGV
      background(backColor);
      stroke(strokeColor);
      strokeWeight(9);
      fill(fillColor);
      rect(0, 0, width, 180);
      fill(strokeColor);
      text("Select your AGV:", 540, 90);
      for (int i = 0; i < names.length; i++) {
        fill(fillColor);
        rect(90, 250 + i * 290, 900, 200, 10);
        fill(strokeColor);
        text(names[i], width / 2, 350 + i * 290);
      }
      if (lastMouse.x >= 90 && lastMouse.x <= 990) {
        for (int i = 0; i < names.length; i++) {
          if (lastMouse.y >= 250 + i * 290 && lastMouse.y <= 450 + i * 290) {
            lastMouse.set(0, 0);
            selected = i;
            deviceName = names[i];
            state = 2;
          }
        }
      }
      break;
    case 2:  //Create the connection
      background(backColor);
      stroke(strokeColor);
      strokeWeight(5);
      fill(overColor);
      rect(width / 2 - 300, height / 2 - 160, 600, 320, 10);
      fill(strokeColor);
      text("Connecting", width / 2, height / 2 - 70);
      text("to device...", width / 2, height / 2 + 70);
      bt.connectToDeviceByName(names[selected]);
      state = 3;
      break;
    case 3:  //Wait for a confirmation of connection
      background(backColor);
      stroke(strokeColor);
      strokeWeight(5);
      fill(overColor);
      rect(width / 2 - 300, height / 2 - 160, 600, 320, 10);
      fill(strokeColor);
      text("Connecting", width / 2, height / 2 - 70);
      text("to device...", width / 2, height / 2 + 70);
      break;
    case 4:  //Normal operation
      background(backColor);
      stroke(strokeColor);
      strokeWeight(9);
      fill(fillColor);
      rect(0, 0, width, 180);
      fill(strokeColor);
      textAlign(CENTER, CENTER);
      text(deviceName, width / 2, 90);
      textAlign(LEFT, CENTER);
      image(modeLogo, 25, 205);
      text("Mode:  " + sMode, 250, 305);
      line(0, 430, width, 430);
      image(treeLogo, 25, 455);
      text("Trees counted:  " + sTrees, 250, 555);
      line(0, 680, width, 680);
      image(batteryLogo, 25, 705);
      text("Battery voltage:  " + sBattery, 250, 805);
      line(0, 930, width, 930);
      break;
    default:
      //Do nothing
      break;
  }
}

void send(int _data) {
  byte[] data = {byte(_data)};
  bt.broadcast(data);
}

void mouseReleased() {
  lastMouse.set(mouseX, mouseY);
}

void onBluetoothDataEvent(String who, byte[] _data) {
  byte data = _data[0];
  switch (state) {
    case 3:
      if (data == 0x57) {
        state = 4;
        send(0x58);
      }
      break;
    case 4:
      if (data <= -1 && data >= -50) {//Values -50 to -1 are battery voltages
        sBattery = str((150 + data) / 10) + "," + str((150 + data) % 10);
      } else if (data <= 51 && data >= 1) {//Values 1 to 51 are numbers of trees (-1)
        sTrees = str(data - 1);
      }
      break;
    default:
      //Do nothing
      break;
  }
}
