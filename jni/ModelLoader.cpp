
#include "string.h"
#include "stdlib.h"
#include "ChunkCodes.h"
#include "DataStructures.h"
#include "ModelLoader.h"


ModelLoader::ModelLoader(void){
}


ModelLoader::~ModelLoader(void){
	
}



void ModelLoader::loadModel(char* f, Model* model){

	struct ChunkSt chn ;
	FILE* file = fopen(f, "rb");
	//read first chunk
    readCnk(file, &chn);
	//parse all other chunks
    parseCnk(model,file,&chn);
    fclose(file);
	
}



 //function to read a ztring
int ModelLoader::getString(char* s,FILE *fp){

	int index = 0;
    char tmp[100] = {0};
    fread(tmp,1,1,fp);
    while(*(tmp + index++)!= 0){
		fread(tmp + index, 1, 1, fp);
    }
    strcpy(s, tmp);
    return (int)(strlen(tmp) + 1);
}


//function to read texture coordinates data
void ModelLoader::readTxtCoord(Model* ob,FILE *fp, struct ChunkSt* chn){
	int numVert = 0;
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);

	//get number of vertices wtih this texture
    chn->bytesRead += fread(&numVert,1,2,fp); 
	//allocate and read in textured vertices
	m->tex =new myTexture[numVert];
    chn->bytesRead += fread((void*)m->tex, 1, numVert*sizeof(struct myTexture), fp);
	
	std::vector<float> v;
	//convert to array of floats
	for(int x=0; x<numVert; x++){    
		 v.push_back(m->tex[x].tu);
		 v.push_back(m->tex[x].tv);	
    }
	int s=v.size();
	m->textureArray = new float[s];
	for(int i=0;i<s;i++){
		 m->textureArray[i]=v[i];
	}

	skipCnk(fp,chn);
}


//function to read the vertices in the object
void ModelLoader::readVertices(Model* ob,FILE *fp, struct ChunkSt* chn){

	int noVert = 0;
    chn->bytesRead += fread(&noVert, 1, 2, fp);  
 
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
	m->verts = new Vertice[noVert];
	m->numVert=noVert;
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += fread((void *)m->verts, 1, noVert*sizeof( struct Vertice), fp);
	//get the total number of vertices
	m->totalNumVerts=m->numVert*3;

	skipCnk(fp, chn);
}

//function to read the faces in the object
void ModelLoader::readFaces(Model* ob,FILE *fp, struct ChunkSt* chn){

	int noFaces = 0;
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
    
    chn->bytesRead += (unsigned int)fread(&noFaces, 1, 2, fp);  
	m->numFace=noFaces;
	m->faces = new Face[noFaces];
	
	//read in faces from the file and increment chunk bytes read
	chn->bytesRead += (unsigned int)fread(m->faces, 1, noFaces*sizeof( struct Face), fp);

	//get the total number of vertices
	m->totalNumFace=m->numFace*3;

	//get normals for this mesh
	getNormals(m);
	//read face material informatin from sub-chunk
	parseCnk(ob,fp,chn);
}

//function to skip past a chunk in the file to the start of the next chunk
void ModelLoader::skipCnk(FILE *fp, struct ChunkSt* chn){

	int buf[50000] ={0};
	fread(buf, 1, chn->size - chn->bytesRead, fp);
}


//function to read a chunk of file
void ModelLoader::readCnk(FILE *fp, struct  ChunkSt* chn){

	unsigned int read=0;
	//read in first 6 bytes -first two are chunk id, other 4 are chunk size
	read = fread(&chn->id, 1, 2, fp);
	read += fread(&chn->size, 1, 4, fp);
	chn->bytesRead=read;
}

//function to read object name from chunk 0x4000
void ModelLoader::readObjectName(Model* ob,FILE *fp, struct ChunkSt* chn){
	Mesh* m = &(ob->meshs[ob->numMeshs-1]);
	//read object name
	int characterlen = getString(m->meshName,fp);
    chn->bytesRead += characterlen;
	parseCnk(ob,fp,chn);
}


//function to parse a chunk nased on its ID
 void ModelLoader::parseCnk(Model* ob,FILE *fp,struct ChunkSt* chn){

      while(chn->bytesRead < chn->size){
            struct ChunkSt tmp;
            readCnk(fp, &tmp);
#ifdef DEBUG
            printCnkInfo(&tmp);
#endif
			Mesh m;
			switch( tmp.id){
				//chunk wil all object data ,eg materials, vertices ect
			    case EDIT3DS:
					 //recursive call to parse internal chunks of EDIT3DS chunk
					 parseCnk(ob,fp,&tmp);
					 break;
				// Object section
			    case NAMED_OBJECT: 
					// Mesh m;
					 //add mesh to object
					 ob->meshs.push_back(m);
					 ob->numMeshs++;
				     readObjectName(ob,fp,&tmp);
					 break;
				//object mesh chunk
				case OBJ_MESH:
					 //recursive call to parse vertices and faces
					 parseCnk(ob,fp,&tmp);
					 break;
				case MESH_VERTICES:
					 readVertices(ob,fp,&tmp);
					 break;
			    case MESH_FACES:
					 readFaces(ob,fp,&tmp);
                     break;
				//texture coordinates mapping
				case MESH_TEX_VERT:
					 readTxtCoord(ob,fp,&tmp);
					 break;
				default:
					 skipCnk(fp, &tmp);
            }
            chn->bytesRead += tmp.size;
      }
}



//function to calcute normals array
void ModelLoader::getNormals(Mesh* m){

	for(int x=0;x<m->numFace;x++){
		Extra* e1=new  Extra;
		e1->vertice=m->verts[m->faces[x].p1];

		Extra* e2=new  Extra;
		e2->vertice=m->verts[m->faces[x].p2];

		Extra* e3=new  Extra;
		e3->vertice=m->verts[m->faces[x].p3];

		//calculate edge vectors (p2-p1, p3-p1)
		float ev1x=m->verts[m->faces[x].p2].x - m->verts[m->faces[x].p1].x;
		float ev1y=m->verts[m->faces[x].p2].y - m->verts[m->faces[x].p1].y;
		float ev1z=m->verts[m->faces[x].p2].z - m->verts[m->faces[x].p1].z;

		float ev2x=m->verts[m->faces[x].p3].x - m->verts[m->faces[x].p1].x;
		float ev2y=m->verts[m->faces[x].p3].y - m->verts[m->faces[x].p1].y;
		float ev2z=m->verts[m->faces[x].p3].z - m->verts[m->faces[x].p1].z;

		//calculate normal vector (N=v1*v2)
		float nvx=(ev1y*ev2z)-(ev1z*ev2y);
		float nvy=(ev1z*ev2x)-(ev1x*ev2z);
		float nvz=(ev1x*ev2y)-(ev1y*ev2x);

		//normalise
		float invlen = 1.0f/sqrt(nvx*nvx+nvy*nvy+nvz*nvz);
		nvx *= invlen;
		nvy *= invlen;
		nvz *= invlen;

	
		//store 
		Vertice* n=new  Vertice;
		n->x=nvx;
		n->y=nvy;
		n->z=nvz;
		e1->normal=*n;
		e2->normal=*n;
		e3->normal=*n;
		
		m->extr.push_back(*e1);
		m->extr.push_back(*e2);
		m->extr.push_back(*e3);
	
	}


}