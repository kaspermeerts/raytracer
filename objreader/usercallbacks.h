#ifndef __USERCALLBACKS_H
#define __USERCALLBACKS_H

#include <stddef.h>


typedef int (*AddVertexFunc)(float x, float y, float z, float weight, 
			     void *userData);
typedef int (*AddTexelFunc)(float x, float y, void *userData);
typedef int (*AddNormalFunc)(float x, float y, float z, void *userData);
typedef int (*StartLineFunc)(void *userData);
typedef int (*AddToLineFunc)(int v, size_t vt, void *userData);
typedef int (*StartFaceFunc)(void *userData);
typedef int (*AddToFaceFunc)(size_t v, size_t vt, size_t vn, void *userData);
typedef int (*StartObjectFunc)(char *name, void *userData);
typedef int (*StartGroupFunc)(void *userData);
typedef int (*AddGroupNameFunc)(char *name, void *userData);
typedef int (*RefMaterialLibFunc)(char *filename, void *userData);
typedef int (*UseMaterialFunc)(char *material, void *userData);
typedef int (*SmoothingGroupFunc)(size_t groupid, void *userData);

typedef struct tagObjParseCallbacks
{
  AddVertexFunc onVertex;
  AddTexelFunc onTexel;
  AddNormalFunc onNormal;
  StartLineFunc onStartLine;
  AddToLineFunc onAddToLine;
  StartFaceFunc onStartFace;
  AddToFaceFunc onAddToFace;
  StartObjectFunc onStartObject;
  StartGroupFunc onStartGroup;
  AddGroupNameFunc onGroupName;
  RefMaterialLibFunc onRefMaterialLib;
  UseMaterialFunc onUseMaterial;
  SmoothingGroupFunc onSmoothingGroup;
  void *userData;
} ObjParseCallbacks;


typedef int (*AddMaterialFunc)(char *name, void *userData);
typedef int (*SetAmbientColorFunc)(float r, float g, float b, void *userData);
typedef int (*SetDiffuseColorFunc)(float r, float g, float b, void *userData);
typedef int (*SetSpecularColorFunc)(float r, float g, float b, void *userData);
typedef int (*SetSpecularExponentFunc)(float se, void *userData);
typedef int (*SetOpticalDensityFunc)(float d, void *userData);
typedef int (*SetAlphaFunc)(float a, void *userData);
typedef int (*SetIllumModelFunc)(int model, void *userData);
typedef int (*SetAmbientTextureMapFunc)(char *path, void *userData);
typedef int (*SetDiffuseTextureMapFunc)(char *path, void *userData);

typedef struct tagMtlParseCallbacks
{
  AddMaterialFunc onAddMaterial;
  SetAmbientColorFunc onSetAmbientColor;
  SetDiffuseColorFunc onSetDiffuseColor;
  SetSpecularColorFunc onSetSpecularColor;
  SetSpecularExponentFunc onSetSpecularExponent;
  SetOpticalDensityFunc onSetOpticalDensity;
  SetAlphaFunc onSetAlpha;
  SetIllumModelFunc onSetIllumModel;
  SetAmbientTextureMapFunc onSetAmbientTextureMap;
  SetDiffuseTextureMapFunc onSetDiffuseTextureMap;
  void *userData;
} MtlParseCallbacks;

#endif /* __USERCALLBACKS_H */
