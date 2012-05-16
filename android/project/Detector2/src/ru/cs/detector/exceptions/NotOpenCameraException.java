package ru.cs.detector.exceptions;

/**
 * Author: Aleksey Vladiev (Avladiev2@gmail.com)
 */
public class NotOpenCameraException extends RuntimeException {
    public NotOpenCameraException() {
        super();
    }

    public NotOpenCameraException(final String message) {
        super(message);
    }

    public NotOpenCameraException(final String message, final Throwable cause) {
        super(message, cause);
    }

    public NotOpenCameraException(final Throwable cause) {
        super(cause);
    }
}
