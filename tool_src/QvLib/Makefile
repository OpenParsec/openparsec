# QvLib makefile

CFILES = QvChildList.cpp\
QvCone.cpp\
QvCoordinate3.cpp\
QvCube.cpp\
QvCylinder.cpp\
QvDb.cpp\
QvDebugError.cpp\
QvDict.cpp\
QvDirectionalLight.cpp\
QvElement.cpp\
QvField.cpp\
QvFieldData.cpp\
QvGroup.cpp\
QvIndexedFaceSet.cpp\
QvIndexedLineSet.cpp\
QvInfo.cpp\
QvInput.cpp\
QvLevelOfDetail.cpp\
QvLists.cpp\
QvMFColor.cpp\
QvMFFloat.cpp\
QvMFLong.cpp\
QvMFVec2f.cpp\
QvMFVec3f.cpp\
QvMaterial.cpp\
QvMaterialBinding.cpp\
QvMatrixTransform.cpp\
QvName.cpp\
QvNode.cpp\
QvNormal.cpp\
QvNormalBinding.cpp\
QvOrthographicCamera.cpp\
QvPList.cpp\
QvPerspectiveCamera.cpp\
QvPointLight.cpp\
QvPointSet.cpp\
QvReadError.cpp\
QvRotation.cpp\
QvSFBitMask.cpp\
QvSFBool.cpp\
QvSFColor.cpp\
QvSFEnum.cpp\
QvSFFloat.cpp\
QvSFImage.cpp\
QvSFLong.cpp\
QvSFMatrix.cpp\
QvSFRotation.cpp\
QvSFString.cpp\
QvSFVec2f.cpp\
QvSFVec3f.cpp\
QvScale.cpp\
QvSeparator.cpp\
QvShapeHints.cpp\
QvSphere.cpp\
QvSpotlight.cpp\
QvState.cpp\
QvString.cpp\
QvSwitch.cpp\
QvTexture2.cpp\
QvTexture2Transform.cpp\
QvTextureCoordinate2.cpp\
QvTransform.cpp\
QvTransformSeparator.cpp\
QvTranslation.cpp\
QvTraverse.cpp\
QvUnknownNode.cpp\
QvWWWAnchor.cpp\
QvWWWInline.cpp


CFLAGS = -g -O2 -I. -I../BspLib -fno-for-scope\
         -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp

all: libqv.a

libqv.a: $(CFILES:.cpp=.o)
	ar rc libqv.a $(CFILES:.cpp=.o)
	ranlib libqv.a

.cpp.o:
	gcc $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a *~




