#ifndef CMDENUM_H
#define CMDENUM_H

enum CmdEnum : int {
    TakePhoto = 1,
    StartVideo,
    EndVideo,
    ChangeAltitudeUp,
    ChangeAltitudeDown,
    Drop,
    None = 0
};

#endif // CMDENUM_H
