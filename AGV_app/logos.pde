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
}
