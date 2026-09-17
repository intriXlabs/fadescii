#include "../fadescii_absloute_zero_version.cpp"

int main() {
    fadescii f;
    f.initialize(5, 10, 10, 50, 30, 100);
    f.setSpeed(0.05);
    f.setGlowLength(10);
    f.setTextColor("#00FF00");
    f.setGlowColor("rgb(255, 0, 0)");
    f.drawText("Hello, World!\nThis is a test of the fadescii library.\nEnjoy the glow effect!");
    return 0;
}