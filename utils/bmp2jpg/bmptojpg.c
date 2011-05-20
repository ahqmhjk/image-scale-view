/* bmptojpg.c*/
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "bmp2jpg.h"
#include <string.h>

JSAMPLE * imagebuffer;
JSAMPLE * image_buffer_no;
JSAMPLE * image_buffer;		/* Points to large array of R,G,B-order data */
int image_height;				/* Number of rows in image */ 
int image_width;				/* Number of columns in image */

/*The function for compress the bmp of 24-BIT-RGB Full-color or 256-grey.*/
size_t bmp2jpg(char *filename,char *outfilename, int quality)
{
  	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	size_t sizenum=0;
  	FILE* tempfile;				/* target file */
	FILE* outfile;
	FILE* infile;
	char* tempfilename="temp";
 	JSAMPROW row_pointer[1];		/* pointer to JSAMPLE row[s] */
  	int row_stride,aaa=0,ccc=0,ddd;	/* physical row width in image buffer */
	size_t bbb=0;
	unsigned long clrused;
	char bitcount=0;
	char addbit,buf[1];
  	unsigned long i=0;
    unsigned char *readdata;
	
	/*typedef unsigned char JSAMPLE,and this malloc for bmp fileheader.*/
	image_buffer=(JSAMPLE*)malloc(54*sizeof(JSAMPLE));

	cinfo.err = jpeg_std_error(&jerr);
  	jpeg_create_compress(&cinfo);

	infile=fopen(filename,"rb");
	if (!infile) 
	{
		free(image_buffer);
    	return OPEN_BMP_FAIL;
  	}	
	if((fread(image_buffer,1,54,infile))!=54)
	{
		fclose(infile);
		free(image_buffer);
		return READ_BMP_FAIL;
	}
	if(image_buffer[0]!=0x42||image_buffer[1]!=0x4d)	/*BMP flag*/
	{
		fclose(infile);
		free(image_buffer);
		return BMP_FORMAT_ERROR;
	}
	
	image_width=(image_buffer[18]+image_buffer[19]*256+image_buffer[20]*256*256+image_buffer[21]*256*256*256);
    image_height=(image_buffer[22]+image_buffer[23]*256+image_buffer[24]*256*256+image_buffer[25]*256*256*256);

	/*24bit RGB BMP file*/
	if((image_buffer[28]+image_buffer[29]*256)==24)
	{
		if((image_width*3)%4==0)
        	addbit=0;
        else
           	addbit=4-((image_width*3)%4);
		sizenum=((image_width*3+addbit)*image_height);
		bitcount=24;
	}
	/*8bit 256 grey scale BMP file*/
	else if((image_buffer[28]+image_buffer[29]*256)==8)
	{
		if(image_width%4==0)
	    	addbit=0;
        else
            addbit=4-(image_width%4);

		clrused=(image_buffer[46]+image_buffer[47]*256+image_buffer[48]*256*256+image_buffer[49]*256*256*256);
		sizenum=((image_width+addbit)*image_height);
		bitcount=8;
	}
	else {
		fclose(infile);
		free(image_buffer);
		return NOT_SUPPORT;	/*OTHER BMP FORMAT AND NOT SUPPORT*/
	}
	free(image_buffer);
	
	image_buffer=(JSAMPLE*)malloc(sizenum);
	image_buffer_no=(JSAMPLE*)malloc(sizenum);
	
	if(bitcount==24)
		fseek(infile,54,SEEK_SET);
	else if(bitcount==8) 
		fseek(infile,(54+clrused*4),SEEK_SET);
	if(fread(image_buffer,1,sizenum,infile)==0)
	{
   		free(image_buffer);
		fclose(infile);
           	return READ_BMP_FAIL;
	}
	/*Change the arrange of per-pixel,and add zero in the end of row 
	if the length of row isn't the multiple of four */
	for(aaa=0,bbb=0;bbb<sizenum;)
	{
		if(bitcount==24)
		{
			ccc=(image_width*3+addbit);
			for(;ccc>addbit;)	
			{
				for(ddd=3;ddd>0;ddd--,aaa++)
					image_buffer_no[aaa]=image_buffer[sizenum-bbb-ccc+ddd-1];
				ccc-=3;
			}
			for(ccc=0;ccc<addbit;aaa++,ccc++)
				image_buffer_no[aaa]=0;
			bbb+=(image_width*3+addbit);
		}
		else if(bitcount==8)
		{
			ccc=(image_width+addbit);
			for(;ccc>0;ccc--,aaa++)
				image_buffer_no[aaa]=image_buffer[sizenum-bbb-ccc];
			bbb+=(image_width+addbit);
		}
	}
	fclose(infile);

	tempfile=fopen(tempfilename,"wb");
	if(!tempfile)
	{
		free(image_buffer);
		free(image_buffer_no);
		return OPEN_BMP_FAIL;			/*Can't open the file again!*/
	}
	jpeg_stdio_dest(&cinfo, tempfile);

  	cinfo.image_width = image_width; /* image width and height, in pixels */
  	cinfo.image_height = image_height;
  	if(bitcount==8)
	{
		cinfo.input_components = 1;		/* # of color components per pixel */
  		cinfo.in_color_space = JCS_GRAYSCALE;	
		/* colorspace of input image JCS_CMYK JCS_YCCK JCS_YCbCr JCS_GRAYSCALE*/
	}
	else if(bitcount==24)
	{
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
		cinfo.input_components = 3;        /* # of color components per pixel */
	}
  	
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,quality,TRUE); 	/* limit to baseline-JPEG values */
	jpeg_start_compress(&cinfo, TRUE);
	if(bitcount==24)
		row_stride = image_width*3+addbit;	/*JSAMPLEs per row in image_buffer*/
	else if(bitcount==8)
		row_stride=image_width+addbit;

  	while (cinfo.next_scanline <cinfo.image_height) 
	{
		row_pointer[0] = &image_buffer_no[cinfo.next_scanline*row_stride];		
		(void)jpeg_write_scanlines(&cinfo,row_pointer,1);
  	}
	fflush(tempfile);
  	jpeg_finish_compress(&cinfo);
  	jpeg_destroy_compress(&cinfo);
	fclose(tempfile);

	/*If no tempfile,the outfile may can't be displayed normally,
	and the reason is still unknow now.*/
	tempfile=fopen(tempfilename,"rb");
	for(;;i++)
	{
		fread(buf,1,1,tempfile);
		if(feof(tempfile)==1)
			break;
	}

	readdata=(unsigned char*)malloc(i);

	fseek(tempfile,0,SEEK_SET);
	fread(readdata,1,i,tempfile);

	outfile=fopen(outfilename,"wb");
	fwrite(readdata,1,i,outfile);
	fclose(outfile);
	fclose(tempfile);
	remove(tempfilename);
	free(readdata);
	free(image_buffer);
	free(image_buffer_no);
	return SUCCESS;
}
#if 0
struct my_error_mgr
{
  	struct jpeg_error_mgr pub;		
  	jmp_buf setjmp_buffer;			
};

typedef struct my_error_mgr *my_error_ptr;

void my_error_exit(j_common_ptr);
void my_error_exit(j_common_ptr cinfo)
{
  	my_error_ptr myerr = (my_error_ptr) cinfo->err;
  	(*cinfo->err->output_message) (cinfo);
  	longjmp(myerr->setjmp_buffer, 1);
}
/*This function for uncompress the jpg of 256-GREY or 24-BIT-RGB Full Color*/
size_t read_JPEG_file(char*,char*);
size_t read_JPEG_file (char *filename,char *outfilename)
{
  	struct jpeg_decompress_struct cinfo;
  	struct my_error_mgr jerr;
	unsigned char* buf;
    unsigned char* buf_no;
	int aaa=0,bbb=0,ccc=0,ddd,i=0,j,k;
	unsigned char head[1078],temp[4],bmpcolor;
  	FILE * infile;				
  	JSAMPARRAY buffer;			
  	int row_stride,addnum,size=0;				

  	if ((infile = fopen(filename, "rb")) == NULL) 
    		return OPEN_JPG_FAIL;    			

  	imagebuffer=(JSAMPLE*)malloc(2);
	if(fread(imagebuffer,1,2,infile)!=2)
	{
		free(imagebuffer);
		fclose(infile);
		return READ_JPG_FAIL;			
	}
	if(imagebuffer[0]!=0xff||imagebuffer[1]!=0xd8)	/*JPG FILE HEAD FLAG*/
	{
		free(imagebuffer);
        fclose(infile);
        return JPG_FORMAT_ERROR;	 	
	}

	fseek(infile,0,SEEK_SET);
	cinfo.err = jpeg_std_error(&jerr.pub);
  	jerr.pub.error_exit = my_error_exit;

  	if (setjmp(jerr.setjmp_buffer)) 
	{
    		jpeg_destroy_decompress(&cinfo);
    		fclose(infile);
    		return JPG_CODE_ERROR;			
  	}
  	jpeg_create_decompress(&cinfo);
  	jpeg_stdio_src(&cinfo, infile);
  	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width *cinfo.output_components;
	
	buf=(unsigned char*)malloc(row_stride*cinfo.output_height);

	if((addnum=row_stride%4)!=0)
		row_stride=(row_stride+4-addnum);
	size=row_stride*cinfo.output_height;
	buf_no=(unsigned char*)malloc(size);
	if(addnum)
		row_stride-=(4-addnum);
	
  	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  	while (cinfo.output_scanline < cinfo.image_height) 
    {
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);
		for(j=0;j<row_stride;j++)
		{
			buf[i]=*(buffer[0]+j);
			i++;
		}
  	}
	k=i;
	fclose(infile);

	infile=fopen(outfilename,"wb+");
	if(!infile)
	{
		free(buf);
		free(buf_no);
		return OPEN_OUTFILE_FAIL;		
	}

	memset(head,'\0',1078);
	memset(temp,'\0',4);
	if(cinfo.output_components==1)/*256 GREY*/
	{
		size=54+1024+size;
		head[10]=0x36;
        head[11]=0x04;
		head[12]=head[13]=0;//FIXME
		head[28]=0x08;
       	head[46]=0x00;//FIXME
		head[47]=0x01;
		head[48]=0x00;//FIXME
		head[49]=0x00;
		head[50]=0x00;//FIXME
		head[51]=0x01;
		head[52]=0x00;//FIXME
		head[53]=0x00;//FIXME
	
		for(bmpcolor=0,i=0;i<1024;bmpcolor++)
		{
			head[54+i]=head[55+i]=head[56+i]=bmpcolor;
			head[57+i]=0xff;
			i+=4;
		}
	}
	else if(cinfo.output_components==3)	/*24 bit RGB Full Color*/
	{
		head[10]=0x36;
		head[11]=head[12]=head[13]=0;//FIXME
		size+=54;
		head[28]=0x18;
		/*for(j=46;j<54;j++)
                	head[j]=0;*/
	}
	head[0]=0x42;
	head[1]=0x4d;
	memcpy(temp,&size,4);
	for(i=2,j=0;j<4;j++,i++)
		head[i] = temp[j];
	head[6]=head[7]=head[8]=head[9]=0;//FIXME
 	head[14]=0x28;
    head[15]=head[16]=head[17]=0;//FIXME
	memcpy(buffer,&cinfo.output_width,4);
	for(i=18,j=0;j<4;j++,i++)
		head[i] = temp[j];
	memcpy(buffer,&cinfo.output_height,4);
	for(i=22,j=0;j<4;j++,i++)
		head[i] = temp[j];
	head[26]=0x01;
	head[27]=head[29]=0x00;//FIXME
	
/*	for(j=30;j<46;j++)
		head[j]=0;*/

	for(;bbb<k;bbb+=row_stride) {
		if(cinfo.output_components==3) {
        	for(ccc=row_stride;ccc>0;ccc-=3) {
            	for(ddd=3;ddd>0;ddd--,aaa++)
                	buf_no[aaa]=buf[k-bbb-ccc+ddd-1];
            }
		}
		else if(cinfo.output_components==1) {
			for(ccc=row_stride;ccc>0;ccc--,aaa++)
				buf_no[aaa]=buf[k-bbb-ccc];
		}
		if(addnum) {
			for(i=0;i<(4-addnum);i++,aaa++)
				buf_no[aaa]=0;
		}
    }
	if(cinfo.output_components==3)
	{
		fwrite(head,1,54,infile);
		fseek(infile,54,SEEK_SET);
	}
	else if(cinfo.output_components==1)
	{
		fwrite(head,1,1078,infile);
                fseek(infile,1078,SEEK_SET);
	}
	fwrite(buf_no,1,aaa,infile);
	
	jpeg_finish_decompress(&cinfo);
  	jpeg_destroy_decompress(&cinfo);

	free(buf);
	free(buf_no);
  	free(imagebuffer);
	fclose(infile);
	return SUCCESS;
}


int main()
{
//	write_JPEG_file("/home/hjk/11.bmp", "/home/hjk/laoren.jpg", 80);
	read_JPEG_file("/home/hjk/laoren.jpg", "/home/hjk/laoren.bmp");
	return 0;
}
#endif

