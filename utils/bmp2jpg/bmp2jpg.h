/*This is for the bmptojpg.h*/
#ifndef _BMPTOJPG_H
#define _BMPTOJPG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*The function: compress bmp (256_grey or RGB)file(filename) to jpg file(outfile),
  according to the define quality */
size_t bmp2jpg(char *filename,char *outfilename, int quality);

/*The function: decompress jpg (256_grey or RGB)file(filename) to bmp file(outfilename)*/
//size_t read_JPEG_file(char *filename,char *outfilename);

/*Return value List*/
/*      --write_JPEG_file--    */
#define 	SUCCESS 		0
#define 	OPEN_BMP_FAIL		1
#define		READ_BMP_FAIL		2
#define		BMP_FORMAT_ERROR	3
#define 	NOT_SUPPORT		4

/*      --read_JPEG_file--     */
#define 	SUCCESS			0
#define 	OPEN_JPG_FAIL		1
#define		READ_JPG_FAIL		2
#define		JPG_FORMAT_ERROR	3
#define		JPG_CODE_ERROR		4
#define 	OPEN_OUTFILE_FAIL	5

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
