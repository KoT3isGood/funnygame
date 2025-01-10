#include "model.h"
#include "stdio.h"
#include "stdlib.h"
void objtobmf(const char* file) {
  FILE* f = fopen(file,"r");
  if (!f) {
    printf("failed to find file\n");
    exit(1);
  }
	uint32_t size = 0;
  fseek(f, 0, SEEK_END); // seek to end of file
  size = ftell(f); // get current file pointer
  fseek(f, 0, SEEK_SET); // seek back to beginning of file
  char* fileData = (char*)malloc(size+1);
  fread(fileData, sizeof(char), size, f); 
  if (size==0) return;
	uint32_t vertices = 0;
	uint32_t uvs = 0;
	uint32_t normals = 0;
	uint32_t indicies = 0;
	for (uint32_t i = 0; i < size; i++) {
		if ((i + 1) > size) break; // file ending

		if (fileData[i] == '\n' && fileData[i+1] == 'v'&& fileData[i + 2] == ' ') {
			vertices++;
		};
		if (fileData[i] == '\n' && fileData[i + 1] == 'v' && fileData[i + 2] == 't' && fileData[i + 3] == ' ') {
			uvs++;
		};
		if (fileData[i] == '\n' && fileData[i + 1] == 'f' && fileData[i + 2] == ' ') {
			indicies++;
		};
	}
	printf("Vertices: %i\n", vertices);
	printf("UVs: %i\n", uvs);
	printf("Indexes: %i\n", indicies);

	uint32_t startPointer = 0;
	uint32_t endPointer = 0;

	void* vertexBufferAlloc = malloc(vertices*12);
	void* uvBufferAlloc = malloc(uvs*8);
	void* indexBufferAlloc = malloc(indicies*12);
	void* uvIndexBufferAlloc = malloc(indicies*12);

	uint32_t amountOfIndicies = indicies * 3;
	uint32_t amountOfVertices = vertices * 3;

	vertices = 0;
	uvs = 0;
	indicies = 0;
	for (uint32_t i = 0; i < size; i++) {
		if ((i + 1) > size) break; // file ending

		uint32_t type = 0;
		uint32_t start = i+1;

		if (fileData[i] == '\n' && fileData[i + 1] == 'v' && fileData[i + 2] == ' ') {
			vertices++;
			type = 1;
		};
		if (fileData[i] == '\n' && fileData[i + 1] == 'f' && fileData[i + 2] == ' ') {
			indicies++;
			type = 2;
			
		};
		if (fileData[i] == '\n' && fileData[i + 1] == 'v' && fileData[i + 2] == 't' && fileData[i + 3] == ' ') {
			uvs++;
			type = 3;

		};
    if (type != 0) {
			i++;
			while (fileData[i] != '\n') {
				i++;
			}
			fileData[i] = '\0';
			if (type == 1) {
				float* v1 = (float*)((uint64_t)vertexBufferAlloc + 12 * (vertices - 1));
				float* v2 = (float*)((uint64_t)vertexBufferAlloc + 4 + 12 * (vertices - 1));
				float* v3 = (float*)((uint64_t)vertexBufferAlloc + 8 + 12 * (vertices - 1));
				sscanf((char*)&fileData[start], "v %f %f %f", v1, v2, v3);
			}
			if (type == 2) {
				uint32_t* v1 = (uint32_t*)((uint64_t)indexBufferAlloc + 12 * (indicies - 1));
				uint32_t* v2 = (uint32_t*)((uint64_t)indexBufferAlloc + 4 + 12 * (indicies - 1));
				uint32_t* v3 = (uint32_t*)((uint64_t)indexBufferAlloc + 8 + 12 * (indicies - 1));

				uint32_t* vt1 = (uint32_t*)((uint64_t)uvIndexBufferAlloc + 12 * (indicies - 1));
				uint32_t* vt2 = (uint32_t*)((uint64_t)uvIndexBufferAlloc + 4 + 12 * (indicies - 1));
				uint32_t* vt3 = (uint32_t*)((uint64_t)uvIndexBufferAlloc + 8 + 12 * (indicies - 1));
				uint32_t none = 0;
				sscanf((char*)&fileData[start], "f %i/%i/%i %i/%i/%i %i/%i/%i", v1, vt1, &none, v2, vt2, &none, v3, vt3, &none);
				*v1 -= 1;
				*v2 -= 1;
				*v3 -= 1;
				*vt1 -= 1;
				*vt2 -= 1;
				*vt3 -= 1;
			}
			if (type == 3) {
				float* v1 = (float*)((uint64_t)uvBufferAlloc + 8 * (uvs - 1));
				float* v2 = (float*)((uint64_t)uvBufferAlloc + 4 + 8 * (uvs - 1));
				sscanf((char*)&fileData[start], "vt %f %f", v1, v2);
			}
			fileData[i] = '\n';
			i--;
		}
	};
  uint32_t outputFileNameLen = strlen(file);
  char* outputFileName = (char*)malloc(outputFileNameLen+1);
  strcpy(outputFileName, file);
  const char* ending = ".bmf";
  memcpy(outputFileName+outputFileNameLen-4,ending, 5);

  fclose(fopen(outputFileName, "w"));
  FILE* outputFile = fopen(outputFileName, "ab");
  fwrite(&indicies,1,4,outputFile);
  fwrite(&vertices,1,4,outputFile);
  fwrite(&normals,1,4,outputFile);
  fwrite(&uvs,1,4,outputFile);
  fwrite(indexBufferAlloc,1,indicies*12, outputFile);
  fwrite(uvIndexBufferAlloc,1,indicies*12,outputFile);
  fwrite(vertexBufferAlloc,1,vertices*12,outputFile);
  fwrite(uvBufferAlloc,1,uvs*8,outputFile);



	free(fileData);
  fclose(f);
}
