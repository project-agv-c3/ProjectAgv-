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

int state = 0;
String[] names = new String[0];
color backColor = color(255, 210, 150);
color strokeColor = color(50);
color fillColor = color(245, 152, 2);

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
    case 1:
      background(backColor);
      stroke(strokeColor);
      strokeWeight(9);
      for (int i = 0; i < names.length; i++) {
        fill(fillColor);
        rect(90, 90 + i * 290, 900, 200, 10);
        fill(strokeColor);
        text(names[i], 540, 190 + i * 290);
      }
      break;
  }
}
