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
PGraphics warnLogo;
PGraphics scanLogo;

int state = 0;
int selected = 0;
int emerColor = 255;
int emergency = -6;
int mode = 0;
int angle = 0;
boolean emergencyMode = false;

String[] names = new String[0];
color backColor = color(255, 210, 150);
color strokeColor = color(50);
color fillColor = color(245, 152, 2);
color overColor = color(50, 80);
color emerbackColor = color(20, 190);
PVector lastMouse = new PVector(0, 0);
String deviceName;
String sMode = "Automatic";
String sTrees = "0";
String sBattery = "12,0 V";

void setup() {
  fullScreen();
  orientation(PORTRAIT);
  createLogos();
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
    case 4:
    case 5:  //Emergency
      background(backColor);
      stroke(strokeColor);
      strokeWeight(9);
      fill(fillColor);
      rect(0, 0, width, 180);/////////////////////////////////////////////
      fill(strokeColor);
      textAlign(CENTER, CENTER);
      text(deviceName, width / 2, 90);
      textAlign(LEFT, CENTER);
      image(modeLogo, 25, 205);
      text("Mode:  " + sMode, 250, 305);
      line(0, 430, width, 430);///////////////////////////////////////////
      image(treeLogo, 25, 455);
      text("Trees counted:  " + sTrees, 250, 555);
      line(0, 680, width, 680);///////////////////////////////////////////
      image(batteryLogo, 25, 705);
      text("Battery voltage:  " + sBattery, 250, 805);
      line(0, 930, width, 930);///////////////////////////////////////////
      image(scanLogo, 25, 955);
      if (mode == 0) {
        text("Perform scan", 250, 1055);
        if (lastMouse.y >= 930 && lastMouse.y <= 1180) {
          lastMouse.set(0, 0);
          send(10);
        }
      } else if (mode == 1) {
        text("Scanning...", 250, 1055);
        pushMatrix();
        translate(125, 1055);
        rotate(radians(angle));
        line(0, 0, 0, 90);
        popMatrix();
        angle += 3;
        if (angle >= 360) {
          angle -= 360;
        }
      }
      line(0, 1180, width, 1180);//////////////////////////////////////////
      if (state == 5) {
        strokeWeight(5);
        fill(emerbackColor);
        rect(100, 200, width - 200, height - 400, 10);
        textAlign(CENTER, CENTER);
        emerColor += emergency;
        if (emerColor >= 255) {
          emergency = -6;
        } else if (emerColor <= 255 - 6 * 15) {
          emergency = 6;
        }
        fill(emerColor, 0, 0);
        textSize(90);
        text("Emergency\nwarning!", width / 2, 400);
        text("Emergency\nwarning!", width / 2, height - 400);
        textSize(70);
        fill(220, 0, 0);
        text("The emergency \n button on the \n AGV has been \n pressed. Please go \n and check to \n see if the area \n is safe again.", width / 2, height / 2);
        createWarning(emerColor);
        image(warnLogo, 160, 320);
        image(warnLogo, width - 320, 320);
        image(warnLogo, 160, height - 480);
        image(warnLogo, width - 320, height - 480);
      }
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
      if (data == 7) {
        state = 4;
        send(8);
      }
      break;
    case 4:
      if (data == 101) {
        state = 5;
      } else if (data >= 70 && data <= 100) {//Values 70 to 100 are battery voltages
        sBattery = str((data + 30) / 10) + "," + str((data + 30) % 10) + " V";
      } else if (data >= -128 && data <= -1) {//Values -128 to -1 are numbers of trees (+128)
        sTrees = str(data + 128);
      } else if (data ==  1) {
        mode = 1;
        sMode = "Automatic";
      } else if (data == 2) {
        mode = 2;
        sMode = "Follow";
      } else if (data == 3) {
        mode = 0;
        sMode = "Idle";
      }
      break;
    case 5:
      if (data != 101) {
        state = 4;
      }
      break;
    default:
      //Do nothing
      break;
  }
}

void createLogos() {
  batteryLogo = createGraphics(200, 200);
  batteryLogo.beginDraw();
  batteryLogo.noFill();
  batteryLogo.stroke(strokeColor);
  batteryLogo.strokeWeight(9);
  batteryLogo.rect(30, 50, 140, 120, 3);
  batteryLogo.rect(40, 30, 40, 20, 3);
  batteryLogo.rect(120, 30, 40, 20, 3);
  batteryLogo.line(60, 65, 60, 95);
  batteryLogo.line(45, 80, 75, 80);
  batteryLogo.line(125, 80, 155, 80);
  batteryLogo.endDraw();
  treeLogo = createGraphics(200, 200);
  treeLogo.beginDraw();
  treeLogo.noFill();
  treeLogo.stroke(strokeColor);
  treeLogo.strokeWeight(9);
  treeLogo.translate(100, 200);
  treeLogo.line(0, -180, 40, -130);
  treeLogo.line(40, -130, 20, -130);
  treeLogo.line(20, -130, 60, -80);
  treeLogo.line(60, -80, 40, -80);
  treeLogo.line(40, -80, 70, -40);
  treeLogo.line(70, -40, 10, -40);
  treeLogo.line(10, -40, 10, -20);
  treeLogo.line(10, -20, -10, -20);////
  treeLogo.line(0, -180, -40, -130);
  treeLogo.line(-40, -130, -20, -130);
  treeLogo.line(-20, -130, -60, -80);
  treeLogo.line(-60, -80, -40, -80);
  treeLogo.line(-40, -80, -70, -40);
  treeLogo.line(-70, -40, -10, -40);
  treeLogo.line(-10, -40, -10, -20);
  treeLogo.endDraw();
  modeLogo = createGraphics(200, 200);
  modeLogo.beginDraw();
  modeLogo.noFill();
  modeLogo.stroke(strokeColor);
  modeLogo.strokeWeight(9);
  modeLogo.translate(100, 100);
  modeLogo.ellipse(0, 0, 120, 120);
  modeLogo.ellipse(0, 0, 110, 110);
  modeLogo.ellipse(0, 0, 100, 100);
  modeLogo.fill(strokeColor);
  modeLogo.ellipse(0, 0, 40, 40);
  for (int i = 0; i < 9; i++) {
    modeLogo.beginShape();
    modeLogo.vertex(-6, -80);
    modeLogo.vertex(6, -80);
    modeLogo.vertex(10, -67);
    modeLogo.vertex(-10, -67);
    modeLogo.endShape(CLOSE);
    modeLogo.rotate(radians(40));
  }
  for (int i = 0; i < 3; i++) {
    modeLogo.rect(-5, 0, 10, 55);
    modeLogo.rotate(radians(120));
  }
  modeLogo.endDraw();
  scanLogo = createGraphics(200, 200);
  scanLogo.beginDraw();
  scanLogo.stroke(strokeColor);
  scanLogo.noFill();
  scanLogo.strokeWeight(9);
  scanLogo.translate(100, 100);
  scanLogo.line(-90, 0, 90, 0);
  scanLogo.line(0, -90, 0, 90);
  scanLogo.ellipse(0, 0, 160, 160);
  scanLogo.ellipse(0, 0, 80, 80);
  scanLogo.endDraw();
}

void createWarning(int _color) {
  modeLogo.endDraw();
  warnLogo = createGraphics(160, 160);
  warnLogo.beginDraw();
  warnLogo.stroke(_color, 0, 0);
  warnLogo.strokeWeight(9);
  warnLogo.line(80, 20, 150, 140);
  warnLogo.line(150, 140, 10, 140);
  warnLogo.line(10, 140, 80, 20);
  warnLogo.strokeWeight(13);
  warnLogo.line(80, 55, 80, 100);
  warnLogo.point(80, 120);
  warnLogo.endDraw();
}
