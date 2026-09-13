#include "../fadescii.cpp"

int main(){
    fadescii f;
    f.initialize(0,0,10,100);
    f.setGlowColor("#FF0000"); // Set glow color to red
    f.setTextColor("rgb(0,255,0)"); // Set text color to green
    f.setSpeed(0.01); // Set speed to 0.01
    f.setGlowLength(10); // Set glow length to 10
    f.drawText("Hello, World! This is a test of the fadescii library.");
    return 0;
}