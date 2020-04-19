/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2017                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2009-2013                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <config.h>

#include "elg_rw.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define SCALASCA_DEBUG_MODULE_NAME    EPILOG
#include <UTILS_Debug.h>
#include <UTILS_Error.h>

#include "elg_impl.h"
#include "epk_archive.h"

#if HAVE( LIBZ )
#  include <zlib.h>
#endif   /* HAVE( LIBZ ) */

#if HAVE( SIONLIB )
#  include <sion.h>
#endif   /* HAVE( SIONLIB ) */


#define ELGIN_MODE_FILE      0
#define ELGIN_MODE_BUFFER    1


/*
 *-----------------------------------------------------------------------------
 * Helper functions
 *-----------------------------------------------------------------------------
 */

static void elg_swap(void* buffer, size_t length)
{
  /* length must be an even number */

  char   tmp;
  size_t i;

  if (length > 1)
    for (i = 0; i < length / 2; i++)
      {
	tmp = ((char*) buffer)[i];
	((char*) buffer)[i] = ((char*) buffer)[length - 1 - i];
	((char*) buffer)[length - 1 - i] = tmp;
      }  
}


/*
 *-----------------------------------------------------------------------------
 * Macro functions
 *-----------------------------------------------------------------------------
 */

#define ELGREC_GET_NUM(rec, var)                                                     \
  memcpy(&var, rec->pos, sizeof(var));                                               \
  rec->pos += sizeof(var);                                                           \
  if ((rec->byte_order == ELG_LITTLE_ENDIAN && ELG_BYTE_ORDER == ELG_BIG_ENDIAN) ||  \
      (rec->byte_order == ELG_BIG_ENDIAN && ELG_BYTE_ORDER == ELG_LITTLE_ENDIAN))    \
      elg_swap(&var, sizeof(var))

/*
 *-----------------------------------------------------------------------------
 * ElgRec
 *-----------------------------------------------------------------------------
 */

/* Data type */
struct ElgRec_struct
{
  elg_ui1   type;
  elg_ui1   length;
  elg_ui1   byte_order;
  buffer_t  pos;
  buffer_t  body;
};


void ElgRec_free(ElgRec* rec)
{
  free(rec->body);
  free(rec);
}

/* Reading record header */

elg_ui1 ElgRec_get_type(ElgRec* rec) 
{
  return rec->type;
}

int ElgRec_is_event(ElgRec* rec)
{
  elg_ui1 type = ElgRec_get_type(rec);
  
  return (type >= 100);
} 

int ElgRec_is_attribute(ElgRec* rec)
{
  elg_ui1 type = ElgRec_get_type(rec);
  
  return (type >= ELG_FIRST_ATTR && type <= ELG_LAST_ATTR);
}

elg_ui1 ElgRec_get_length(ElgRec* rec) 
{
  return rec->length;
}

/* Reading record data */

elg_ui1 ElgRec_read_ui1(ElgRec* rec) 
{
  elg_ui1 ui1;

  ELGREC_GET_NUM(rec, ui1);  
  return ui1;
}

elg_ui4 ElgRec_read_ui4(ElgRec* rec) 
{
  elg_ui4 ui4;

  ELGREC_GET_NUM(rec, ui4);  
  return ui4;
}

elg_ui8 ElgRec_read_ui8(ElgRec* rec) 
{
  elg_ui8 ui8;

  ELGREC_GET_NUM(rec, ui8);  
  return ui8;
}

elg_d8 ElgRec_read_d8(ElgRec* rec)
{
  elg_d8 d8;

  ELGREC_GET_NUM(rec, d8);  
  return d8;
}

char* ElgRec_read_string(ElgRec* rec)
{
  return (char*) rec->pos;                   
}


/*
 *-----------------------------------------------------------------------------
 * ElgIn
 *-----------------------------------------------------------------------------
 */

/* Data type */

struct ElgIn_struct  
{
  int            mode;
#if !HAVE( LIBZ )
  FILE*          file;
#else   /* HAVE( LIBZ ) */
  gzFile         file;
#endif   /* HAVE( LIBZ ) */
  const elg_ui1* buffer;
  int            sionid;
  long           bytes_avail_in_block;
  size_t         size;
  size_t         offset;
  char           name[PATH_MAX];
  elg_ui1        minor_vnr; 
  elg_ui1        major_vnr; 
  elg_ui1        byte_order; 
};



/* SION: check if gzread can read next bytes in sionfile */

static int check_sion_next_read(ElgIn* in, int bytes) {
#if !HAVE( SIONLIB )
  return(1);
#else   /* HAVE( SIONLIB ) */
  int bytes_in_gzbuffer=-1;

  if(in->sionid>=0) {  	/* is only set, if file is a sion file, e.g. not for defs */

#if !HAVE( LIBZ )
    if(in->bytes_avail_in_block<0) {
      DPRINTFP((  512,"ELG: check_sion_next_read",-1,"call sion_bytes_avail_in_block bytes=%d\n",bytes));
      in->bytes_avail_in_block=sion_bytes_avail_in_block(in->sionid); 
    }

    if(bytes > in->bytes_avail_in_block) {
      if(in->bytes_avail_in_block==0) {
	DPRINTFP((  512,"ELG: check_sion_next_read",-1,"call sion_feof bytes=%d\n",bytes));
	if(sion_feof(in->sionid)) {
	  DPRINTFP((  512,"ELG: check_sion_next_read",-1,"eof reached\n"));
	  return(0);
	}
	in->bytes_avail_in_block=sion_bytes_avail_in_block(in->sionid); 
      } else {
	  DPRINTFP((  512,"ELG: check_sion_next_read",-1,"something wrong bytes > in->bytes_avail_in_block %ld > %ld\n",bytes,in->bytes_avail_in_block));
	  return(0);
      }
    }
    in->bytes_avail_in_block-=bytes;
#else   /* HAVE( LIBZ ) */
    if(in->bytes_avail_in_block<0) {
      in->bytes_avail_in_block=sion_bytes_avail_in_block(in->sionid); 
      DPRINTFP((  512,"ELG: check_sion_next_read",-1,"call sion_bytes_avail_in_block bytes_in_gzbuffer=%d bytes=%d bytes_avail_in_block=%d result\n",
		  bytes_in_gzbuffer,bytes,in->bytes_avail_in_block));
    }
    if(in->bytes_avail_in_block<=0) {
      bytes_in_gzbuffer=gzgetbufferavailbytes(in->file); 
      
      if(bytes_in_gzbuffer<bytes) { 
	DPRINTFP((  512,"ELG: check_sion_next_read",-1,"call sion_feof bytes_in_gzbuffer=%d bytes=%d\n",bytes_in_gzbuffer,bytes));
	if(sion_feof(in->sionid)) {
	  if(bytes_in_gzbuffer<=0) {
	    DPRINTFP((  512,"ELG: check_sion_next_read",-1,"eof reached\n"));
	    return(0);
	  }
	}
	in->bytes_avail_in_block=sion_bytes_avail_in_block(in->sionid); 
	DPRINTFP((  512,"ELG: check_sion_next_read",-1,"call sion_bytes_avail_in_block2 bytes_in_gzbuffer=%d bytes=%d bytes_avail_in_block=%d result\n",
		    bytes_in_gzbuffer,bytes,in->bytes_avail_in_block));
      }
    }
#endif   /* HAVE( LIBZ ) */

  }
  return(1);
#endif   /* HAVE( SIONLIB ) */
}

/* Open and close trace file */

ElgIn* ElgIn_open(const char* path_name)
{
  ElgIn*   in;
  char     label[7];
  struct stat sbuf;
  int status; 

#if HAVE( SIONLIB )
  int   open_with_sion=0; 
  int   opened_with_sion=0; 
  int   sion_id=-1; 
  int   sionrank=-1;
  char *sionfname=NULL;
#endif   /* HAVE( SIONLIB ) */

#if HAVE( SIONLIB )
  /* check if file is in a SION file */
 { 
   int rc;
   char *cptr=NULL;

   if ( (cptr=strstr(path_name,"SID="))!=NULL) {
     rc=sscanf(cptr,"SID=%d:",&sion_id);
     opened_with_sion=1;
     open_with_sion=1;
     DPRINTFP((  512,"ELG: ElgIn_open",-1,"path_name= %s  sion_id=%d opened_with_sion=%d\n",path_name,sion_id,opened_with_sion));
   } else {
     
     if ( (cptr=strstr(path_name,"ELG/"))!=NULL) {
       rc=sscanf(cptr,"ELG/%d",&sionrank);
       sionfname=epk_archive_filename(EPK_TYPE_SION,epk_archive_directory(EPK_TYPE_ELG));
       open_with_sion=_sion_stat_file(sionfname);
       DPRINTFP((  512,"ELG: ElgIn_open",-1,"path_name= %s  sionrank=%d sionfname=%s open_with_sion=%d\n",path_name,sionrank,sionfname,open_with_sion));
     }
   }
 }
#endif   /* HAVE( SIONLIB ) */

#if HAVE( SIONLIB )
 if(!open_with_sion) {
#endif   /* HAVE( SIONLIB ) */
  /* check file status */
  status=stat(path_name, &sbuf);
  if (status) {
    UTILS_DEBUG("stat(%s): %s", path_name, strerror(errno));
    return NULL;
  }
  if (sbuf.st_blocks == 0) { /* number of 512-byte blocks allocated */
    UTILS_WARNING("%u blocks allocated for %u bytes of %s",
        sbuf.st_blocks, sbuf.st_size, path_name);
  }

#if HAVE( SIONLIB )
 }
#endif   /* HAVE( SIONLIB ) */

  /* allocate ElgIn record */
  in = (ElgIn*)malloc(sizeof(ElgIn));
  if (in == NULL)
    UTILS_FATAL("Allocation of EPILOG record struct failed");


#if HAVE( SIONLIB )
 /* open SION file */
 if(open_with_sion) {
   FILE *fp, *secondfp=NULL;
   sion_int32 fsblocksize;
   sion_int64 localsize;
   int secondfd;
   if(!opened_with_sion) {
     in->sionid=sion_open_rank( sionfname, "rb", &localsize, &fsblocksize, &sionrank, &fp);
     strcpy(in->name, path_name);
   } else {
     in->sionid=sion_id;
     fp=sion_get_fp(sion_id);
     strcpy(in->name, path_name+10);
   }
   in->mode = ELGIN_MODE_FILE;

#if !HAVE( LIBZ )
   in->file = fp;
#else   /* HAVE( LIBZ ) */
   secondfd=dup(fileno(fp));
   in->file = gzdopen(secondfd, "r");

   secondfp = gzgetfileptr(in->file);
   sion_set_second_fp(in->sionid,secondfp);
   sion_optimize_fp_buffer(in->sionid);
#endif   /* HAVE( LIBZ ) */
   in->bytes_avail_in_block=-1;

   if(in->file==NULL) {
      UTILS_WARNING("Cannot open %s file %s", epk_archive_filetype(path_name), path_name);
      free(sionfname);
      free(in);
      return NULL;
   } else {
     UTILS_DEBUG("Opened %s file %s for reading %d bytes",
		 epk_archive_filetype(path_name), path_name, -1);
   }
   if(sionfname!=NULL) free(sionfname);
 } else {
   /* open single file */
   if(sionfname!=NULL) free(sionfname);
   in->sionid=-1;
   in->bytes_avail_in_block=-1;
#endif   /* HAVE( SIONLIB ) */

  /* open file */
  in->mode = ELGIN_MODE_FILE;
#if !HAVE( LIBZ )
  if ((in->file = fopen(path_name, "r")) == NULL)
#else   /* HAVE( LIBZ ) */
  if ((in->file = gzopen(path_name, "r")) == NULL)
#endif   /* HAVE( LIBZ ) */
    {
      UTILS_WARNING("Cannot open %s file %s", epk_archive_filetype(path_name), path_name);
      free(in);
      return NULL;
    }
  else
    UTILS_DEBUG("Opened %s file %s for reading %d bytes",
                epk_archive_filetype(path_name), path_name, sbuf.st_size);

  /* store file name */
  strcpy(in->name, path_name);

#if HAVE( SIONLIB )
 }
#endif   /* HAVE( SIONLIB ) */

#if HAVE( SIONLIB )
  /* read header and check file format */ 
 check_sion_next_read(in,strlen(ELG_HEADER) + 1 + sizeof(in->major_vnr) + sizeof(in->minor_vnr) + sizeof(in->byte_order));
#endif   /* HAVE( SIONLIB ) */

#if !HAVE( LIBZ )
  if (fread(label, strlen(ELG_HEADER) + 1, 1, in->file)           != 1 ||
      fread(&in->major_vnr, sizeof(in->major_vnr), 1, in->file)   != 1 ||
      fread(&in->minor_vnr, sizeof(in->minor_vnr), 1, in->file)   != 1 ||
      fread(&in->byte_order, sizeof(in->byte_order), 1, in->file) != 1 ||
      strcmp(label, ELG_HEADER) != 0)
    {
      fclose(in->file);
      free(in);
      return NULL;
    }
#else   /* HAVE( LIBZ ) */
#if !HAVE( SIONLIB )
  if (gzread(in->file, label, strlen(ELG_HEADER) + 1)           != strlen(ELG_HEADER) + 1 ||
      gzread(in->file, &in->major_vnr, sizeof(in->major_vnr))   != sizeof(in->major_vnr) ||
      gzread(in->file, &in->minor_vnr, sizeof(in->minor_vnr))   != sizeof(in->minor_vnr) ||
      gzread(in->file, &in->byte_order, sizeof(in->byte_order)) != sizeof(in->byte_order) ||
      strcmp(label, ELG_HEADER) != 0)
#else   /* HAVE( SIONLIB ) */
  if (gzread_bs(in->file, label, strlen(ELG_HEADER) + 1,&in->bytes_avail_in_block)           != strlen(ELG_HEADER) + 1 ||
      gzread_bs(in->file, &in->major_vnr, sizeof(in->major_vnr),&in->bytes_avail_in_block)   != sizeof(in->major_vnr) ||
      gzread_bs(in->file, &in->minor_vnr, sizeof(in->minor_vnr),&in->bytes_avail_in_block)   != sizeof(in->minor_vnr) ||
      gzread_bs(in->file, &in->byte_order, sizeof(in->byte_order),&in->bytes_avail_in_block) != sizeof(in->byte_order) ||
      strcmp(label, ELG_HEADER) != 0)
#endif   /* HAVE( SIONLIB ) */
    {
      gzclose(in->file);
      free(in);
      return NULL;
    }

#endif   /* HAVE( LIBZ ) */

  return in;  
}


int ElgIn_close(ElgIn* in)
{
  int result;

  if (in->mode == ELGIN_MODE_FILE)
    {
#if !HAVE( LIBZ )
#if HAVE( SIONLIB )
      if(in->sionid>=0) {
	if(sion_is_serial_opened(in->sionid)) result = sion_close(in->sionid);
      } else {
	result = fclose(in->file);
      }
#else   /* !HAVE( SIONLIB ) */
      result = fclose(in->file);
#endif   /* !HAVE( SIONLIB ) */
#else   /* HAVE( LIBZ ) */
      result = gzclose(in->file);
#if HAVE( SIONLIB )
      if(in->sionid>=0) {
	sion_unset_second_fp(in->sionid);
	if(sion_is_serial_opened(in->sionid)) result=sion_close(in->sionid);
      }
#endif   /* HAVE( SIONLIB ) */

#endif   /* HAVE( LIBZ ) */
      if (result == 0)
        UTILS_DEBUG("Closed %s file %s",
                    epk_archive_filetype(in->name), in->name);
    }
  else   /* ELGIN_MODE_BUFFER */
    {
      /* UTILS_DEBUG("Closed buffer"); */
      result=0;
    }

  free(in);

  return result;
}

/* EPILOG version */

EXTERN elg_ui4  ElgIn_get_version(ElgIn* in)
{
  return in->major_vnr * 1000 + in->minor_vnr;
}

/* File position and error management */

int ElgIn_seek(ElgIn* in, long offset)
{
  if (in->mode == ELGIN_MODE_FILE)
    {
#if !HAVE( LIBZ )
      return fseek(in->file, offset, SEEK_SET);
#else   /* HAVE( LIBZ ) */
      return gzseek(in->file, offset, SEEK_SET);
#endif   /* HAVE( LIBZ ) */
    }

  /* ELGIN_MODE_BUFFER */
  if (offset < 0 || (size_t)offset >= in->size)
    return -1;

  in->offset = offset;
  return 0;
}

/* Reading trace records */

ElgRec* ElgIn_read_record(ElgIn* in)
{
  ElgRec* rec;

  /* allocate record */
  rec = calloc(1,sizeof(ElgRec));
  if (rec == NULL)
    UTILS_FATAL("Allocation of EPILOG record struct failed");

  if (in->mode == ELGIN_MODE_FILE)
    {

      /* complete record is stored in a block, 
	 recalulation not necessary for the gzreads of one record  */
      if(!check_sion_next_read(in,sizeof(rec->length)+sizeof(rec->type))) {
          free(rec);
          return NULL;
      }

#if !HAVE( LIBZ )
      if (fread(&rec->length, sizeof(rec->length), 1, in->file) != 1 ||
          fread(&rec->type, sizeof(rec->type), 1, in->file)     != 1 )
#else   /* HAVE( LIBZ ) */
#if !HAVE( SIONLIB )
      if (gzread(in->file, &rec->length, sizeof(rec->length)) != sizeof(rec->length) ||
          gzread(in->file, &rec->type, sizeof(rec->type))     != sizeof(rec->type) )
#else   /* HAVE( SIONLIB ) */
      if (gzread_bs(in->file, &rec->length, sizeof(rec->length),&in->bytes_avail_in_block) != sizeof(rec->length) ||
          gzread_bs(in->file, &rec->type, sizeof(rec->type),&in->bytes_avail_in_block)     != sizeof(rec->type) )
#endif   /* HAVE( SIONLIB ) */
#endif   /* HAVE( LIBZ ) */
        {
          free(rec);

          return NULL;
        }
    }
  else   /* ELGIN_MODE_BUFFER */
    {
      if (in->size - in->offset < sizeof(rec->length) + sizeof(rec->type))
        {
          free(rec);
          return NULL;
        }

      rec->length = in->buffer[in->offset++];
      rec->type   = in->buffer[in->offset++];
    }

  /* allocate record body */
  if (rec->length)
    {
      rec->body = malloc(rec->length);
      if (rec->body == NULL)
        UTILS_FATAL("Allocation of EPILOG record body failed");

      /* read record body */
      if (in->mode == ELGIN_MODE_FILE)
        {
	  /* complete record is stored in a block, 
	     recalulation not necessary for the gzreads of one record  */
	  if(!check_sion_next_read(in,rec->length)) {
              ElgRec_free(rec);
              return NULL;
	  } 

#if !HAVE( LIBZ )
          if (fread(rec->body, rec->length, 1, in->file) != 1)
#else   /* HAVE( LIBZ ) */
#if !HAVE( SIONLIB )
          if (gzread(in->file, rec->body, rec->length) != rec->length)
#else   /* HAVE( SIONLIB ) */
          if (gzread_bs(in->file, rec->body, rec->length, &in->bytes_avail_in_block) != rec->length)
#endif   /* HAVE( SIONLIB ) */
#endif   /* HAVE( LIBZ ) */
            {
              ElgRec_free(rec);
              return NULL;
            }
        }
      else   /* ELGIN_MODE_BUFFER */
        {
          if (in->size - in->offset < rec->length)
            {
              ElgRec_free(rec);
              return NULL;
            }

          memcpy(rec->body, &in->buffer[in->offset], rec->length);
          in->offset += rec->length;
        }
    }
  else
    {
      rec->body = NULL;
    }

  rec->byte_order = in->byte_order;
  rec->pos        = rec->body;

  return rec;
}
