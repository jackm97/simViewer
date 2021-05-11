#ifndef RENDERFUNCS_H
#define RENDERFUNCS_H

void doRendererUpdate();

bool JSSFRender(void* imgPtr);

bool JSSFIterRender(void* imgPtr);

bool LBMRender(void* imgPtr);

bool JSSF3DRender(void* imgPtr);

void renderSims();

#endif