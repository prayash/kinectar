import org.openkinect.freenect.*;
import org.openkinect.processing.*;

Kinect kinect;

float a = 0;
float[] depthLookUp = new float[2048];

void setup() {
  size(800, 600, P3D);
  kinect = new Kinect(this);
  kinect.initDepth();

  // Lookup table for all possible depth values (0 - 2047)
  for (int i = 0; i < depthLookUp.length; i++) {
    depthLookUp[i] = rawDepthToMeters(i);
  }
}


void draw() {
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
  a += 0.0015;
}

// These functions come from: http://graphics.stanford.edu/~mdfisher/Kinect.html
float rawDepthToMeters(int depthValue) {
  if (depthValue < 2047) {
    return (float)(1.0 / ((double)(depthValue) * -0.0030711016 + 3.3309495161));
  }
  return 0.0f;
}

//calculte the xyz camera position based on the depth data
PVector depthToPointCloudPos(int x, int y, float depthValue) {
  PVector point = new PVector();
  // float depth = (float)depthLookUp[depthValue];

  point.z = (float)(depthValue);// / (1.0f); // Convert from mm to meters
  point.x = (float)(x - CameraParams.cx) * point.z / CameraParams.fx;
  point.y = (float)(y - CameraParams.cy) * point.z / CameraParams.fy;
  return point;
}

static class CameraParams {
  static float cx = 3.3930780975300314e+02;
  static float cy = 2.4273913761751615e+02;
  static float fx = 1.0 / 5.9104053696870778e+02;
  static float fy = 1.0 / 5.9421434211923247e+02;
}
