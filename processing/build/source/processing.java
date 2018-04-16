import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import org.openkinect.freenect.*; 
import org.openkinect.processing.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class processing extends PApplet {




Kinect kinect;

float a = 0;
float[] depthLookUp = new float[2048];

public void setup() {
  
  kinect = new Kinect(this);
  kinect.initDepth();

  // Lookup table for all possible depth values (0 - 2047)
  for (int i = 0; i < depthLookUp.length; i++) {
    depthLookUp[i] = rawDepthToMeters(i);
  }
}


public void draw() {
  background(0);

  // Translate and rotate
  pushMatrix();
  translate(width/2, height/2, 0);
  rotateY(a);

  // We're just going to calculate and draw every 2nd pixel
  int skip = 4;

  // Get the raw depth as array of integers
  int[] depth = kinect.getRawDepth();

  stroke(255);
  strokeWeight(2);
  beginShape(POINTS);

  for (int x = 0; x < kinect.width; x += skip) {
    for (int y = 0; y < kinect.height; y += skip) {
      int offset = x + y * kinect.width;
      int d = depth[offset];

      PVector point = depthToPointCloudPos(x, y, d);
      vertex(point.x, point.y, point.z);
    }
  }

  endShape();

  popMatrix();

  fill(255);
  text(frameRate, 50, 50);

  // Rotate
  a += 0.0015f;
}

// These functions come from: http://graphics.stanford.edu/~mdfisher/Kinect.html
public float rawDepthToMeters(int depthValue) {
  if (depthValue < 2047) {
    return (float)(1.0f / ((double)(depthValue) * -0.0030711016f + 3.3309495161f));
  }
  return 0.0f;
}

//calculte the xyz camera position based on the depth data
public PVector depthToPointCloudPos(int x, int y, float depthValue) {
  PVector point = new PVector();
  // float depth = (float)depthLookUp[depthValue];

  point.z = (float)(depthValue);// / (1.0f); // Convert from mm to meters
  point.x = (float)(x - CameraParams.cx) * point.z / CameraParams.fx;
  point.y = (float)(y - CameraParams.cy) * point.z / CameraParams.fy;
  return point;
}

static class CameraParams {
  static float cx = 3.3930780975300314e+02f;
  static float cy = 2.4273913761751615e+02f;
  static float fx = 1.0f / 5.9104053696870778e+02f;
  static float fy = 1.0f / 5.9421434211923247e+02f;
}
  public void settings() {  size(800, 600, P3D); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "processing" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
