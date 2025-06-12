#ifndef VDC_INFO_SCREEN_H
#define VDC_INFO_SCREEN_H

void draw_vdc_info_screen(unsigned char screen_width);
extern unsigned char VDC_value;       // Exposed global from assembly
extern void DetectVDCMemSize(void);   // Exposed function

#endif
