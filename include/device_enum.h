#ifndef DEVICE_ENUM_H
#define DEVICE_ENUM_H

enum TTYDevice {
    USB, ACM
};

enum ResultType {
    LAST_RESULT_ONLY, WHOLE_RESULT
};

enum Mode {
    SINGLE_MODE, MULTI_MODE, DELETE_MODE, INPUT_MODE
};

#endif
