package ru.cs.detector.tools;

/**
 * Author: Aleksey Vladiev (Avladiev2@gmail.com)
 */
public class Rect {
    public final int x;
    public final int y;
    public final int width;
    public final int height;

    public Rect(final int x, final int y, final int width, final int height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    @Override
    public String toString() {
        return "Rect{" +
                "x=" + x +
                ", y=" + y +
                ", width=" + width +
                ", height=" + height +
                '}';
    }
}
