#ifndef __OBJREADER_H
#define __OBJREADER_H

#include <stdio.h>
#include <objreader/usercallbacks.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Read OBJ file into memory and initialize the 
 * content structure
 * @param stream - obj file handle opened for reading
 * @param ucb - structure with user callback function 
 *              pointers which will be called during
 *              parsing
 * @return 0 if successfull, otherwise the error code
*/
int ReadObjFile(FILE *stream, ObjParseCallbacks *ucb);

/**
 * Read MTL file into memory and initialize the 
 * content structure
 * @param stream - obj file handle opened for reading
 * @param ucb - structure with user callback function 
 *              pointers which will be called during
 *              parsing
 * @return 0 if successfull, otherwise the error code
*/
int ReadMtlFile(FILE *stream, MtlParseCallbacks *ucb);


#ifdef  __cplusplus
}
#endif

#endif /* __OBJREADER_H */
