#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES
#include <vector>

//chunk structure
struct ChunkSt {
	unsigned short id;
	unsigned int size;
	unsigned int bytesRead;
};

//structure fo a vertice
struct Vertice{ 
	float x, y, z; 
};
//EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!EXTRA!
struct Extra{
	 Vertice vertice;
	 Vertice normal;
	//struct{ unsigned char r,g,b,a ; }Color;
};




//structure for face (a triangle)
struct Face{ 
	unsigned short p1, p2, p3,matID; 
};
//texture structure
struct myTexture{
	float tu,tv;
};
//material structure
struct Material{
    char name[256];
    struct{ unsigned char r,g,b ; }Color;
    char textureFile[256];
};

//structure for object mesh
struct Mesh{

	char meshName[256];
	int numVert,numFace,totalNumVerts,totalNumFace;
	struct Vertice *verts;
	struct Face *faces;
	struct myTexture *tex;
	float *textureArray;
	std::vector<Extra> extr;
	Mesh(){
		numVert=0;
		numFace=0;
		verts=NULL;
		faces=NULL;
		tex=NULL;
	}
};
//structure for an object conisting of meshes and materials
struct Model{

	int numMeshs,numMat;
	std::vector<Mesh> meshs;
	std::vector<Material> materials;
	
	Model(){
		numMeshs=0;
		numMat=0;
	}
};







#endif