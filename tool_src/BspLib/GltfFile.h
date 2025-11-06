//-----------------------------------------------------------------------------
//	BSPLIB HEADER: GltfFile.h
//
//  Copyright (c) 2025 for glTF support
//  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef _GLTFFILE_H_
#define _GLTFFILE_H_

// bsplib header files
#include "BspLibDefs.h"
#include "BoundingBox.h"
#include "InputData3D.h"
#include <vector>
#include <string>
#include <map>


BSPLIB_NAMESPACE_BEGIN


class BspObjectList;

// file manipulation class for glTF format files ------------------------------
//
class GltfFile : public InputData3D {

public:
	GltfFile( BspObjectList objectlist, const char *filename );
	~GltfFile();

	int				ParseObjectData();
	int				WriteOutputFile();

private:
	// glTF data structures
	struct GltfAccessor {
		int bufferView;
		int byteOffset;
		int componentType;
		bool normalized;
		int count;
		std::string type;
		std::vector<double> min;
		std::vector<double> max;
		std::vector<float> data; // Parsed data
	};

	struct GltfBufferView {
		int buffer;
		int byteOffset;
		int byteLength;
		int byteStride;
		std::string target;
	};

	struct GltfBuffer {
		std::string uri;
		int byteLength;
		std::vector<unsigned char> data;
	};

	struct GltfMeshPrimitive {
		int indices;
		int position;
		int normal;
		int texcoord;
		int material;
		int mode; // 0=POINTS, 1=LINES, 2=LINE_LOOP, 3=LINE_STRIP, 4=TRIANGLES, 5=TRIANGLE_STRIP, 6=TRIANGLE_FAN
	};

	struct GltfMesh {
		std::string name;
		std::vector<GltfMeshPrimitive> primitives;
	};

	struct GltfMaterial {
		std::string name;
		std::string diffuseTexture;
		std::vector<float> diffuseColor;
		float transparency;
	};

	// Parsing methods
	bool			ParseJson(const std::string& json);
	bool			ParseBuffers();
	bool			ParseBufferViews();
	bool			ParseAccessors();
	bool			ParseMeshes();
	bool			ParseMaterials();
	bool			LoadBufferData(const std::string& uri, std::vector<unsigned char>& data);
	bool			LoadEmbeddedData(const std::string& dataUri, std::vector<unsigned char>& data);
	void			ProcessMesh(const GltfMesh& mesh);
	void			ProcessPrimitive(const GltfMeshPrimitive& primitive);
	void			CreateObjectFromMesh(const std::string& name, 
										 const std::vector<Vector3>& vertices,
										 const std::vector<Vector3>& normals,
										 const std::vector<Vector2>& texcoords,
										 const std::vector<int>& indices,
										 int materialIndex);

private:
	std::string m_basePath;
	std::vector<GltfBuffer> m_buffers;
	std::vector<GltfBufferView> m_bufferViews;
	std::vector<GltfAccessor> m_accessors;
	std::vector<GltfMesh> m_meshes;
	std::vector<GltfMaterial> m_materials;
	std::string m_jsonContent;
};


BSPLIB_NAMESPACE_END

#endif // _GLTFFILE_H_
