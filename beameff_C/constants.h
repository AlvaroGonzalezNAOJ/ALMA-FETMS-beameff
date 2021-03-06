#ifndef CONSTANTS_H
#define CONSTANTS_H

extern char *VersionNumber;
extern float c;
extern float c_mm_per_ns;
extern float PI;
extern float subreflector_radius12m;
extern float subreflector_radius7m;

typedef enum {
    NOMINAL,
    ACTUAL,
    ACA_7METER,
    BAND1_TEST
} PointingOptionType;

#endif
