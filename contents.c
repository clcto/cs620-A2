/* 
 * CS620 - A2
 *    list the items in directories passed in at the
 *    command line.
 *
 *  Carick Wienke
 *  2011.02.24
 *
 *  Code was derived from 'lsi1.c' by Scott Valcourt
 *
 *  Code to convert to mode_t to a string is done in
 *  strmode.c. This file is from Apple:
 *  http://www.opensource.apple.com/source/Libc/Libc-166/string.subproj/strmode.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

typedef struct{
   char name[ NAME_MAX ];
   ino_t inode;
   uid_t uid;
   gid_t gid;
   off_t size;
   time_t mtime;
   mode_t mode;
} compiled_stats_t;


void list_dir( char *path );
int stat_cmp( const void*, const void* );
void print_stat( const compiled_stats_t const* );
void strmode( mode_t, char* );

int main( int argc, char *argv[] )
{
   if( argc <= 1 )
      list_dir( "." );
   else
   {
      int i;
      for( i = 1; i < argc; ++i )
         list_dir( argv[i] );
   }

   return EXIT_SUCCESS;
}

void list_dir( char *path )
{
   DIR *directory;
   struct dirent *entry;

   char abs_path[ PATH_MAX ];

      // expand the path to absolute path (if it isn't already)
   char *status = realpath( path, abs_path );

   if( status == NULL ) // true if realpath fails
   {
      perror( path );
      fprintf( stderr, "\n" );
      return;
   }

   directory = opendir( abs_path );
   if( directory == NULL )
   {
      perror( path );
      fprintf( stderr, "\n" );
      return;
   }

      // count the number of files and create an array
      // to store the stats in.
      //
      // this is done this way to conserve memory at the
      // expense of execution time
   int file_count = 0;
   errno = 0;
   while( ( entry = readdir( directory ) ) )
   {
      if( entry->d_name[0] == '.' )
         continue;
      
      ++file_count;
      errno = 0;
   }
   if( errno )
   {
      perror( "readdir" );
      fprintf( stderr, "\n" );
   }
   if( closedir( directory ) )
   {
      perror( "closedir" );
      fprintf( stderr, "\n" );
   }


      // reopen to actually read now
   directory = opendir( abs_path );
   if( directory == NULL )
   {
      perror( path );
      fprintf( stderr, "\n" );
      return;
   }

   compiled_stats_t files[ file_count ];

   int i = 0;
   errno = 0;
   while( ( entry = readdir( directory ) ) )
   {
      if( entry->d_name[0] != '.' )
      { 
            // get the name from the dirent struct
         strcpy( files[ i ].name, entry->d_name );
         
         struct stat entry_stats;
         
            // get the other info from the stat struct 
         char full_name[NAME_MAX];
         sprintf( full_name, "%s/%s", path, files[i].name );
         stat( full_name, &entry_stats );

         files[i].size = entry_stats.st_size;
         files[i].mtime = entry_stats.st_mtime;
         files[i].mode = entry_stats.st_mode;

         files[i].uid = entry_stats.st_uid;
         files[i].gid = entry_stats.st_gid;
         files[i].inode = entry_stats.st_ino;
         
         ++i;
      }
      errno = 0;
   }
   if( errno )
   {
      perror( "readdir" );
      fprintf( stderr, "\n" );
   }
   if( closedir( directory ) )
   {
      perror( "closedir" );
      fprintf( stderr, "\n" );
   }


   qsort( files, file_count, sizeof( compiled_stats_t ),
          stat_cmp );
   
   printf( "%s\n\n", abs_path );
   for( i = 0; i < file_count; ++i )
      print_stat( &files[i] );
}

void print_stat( const compiled_stats_t const * stats )
{
   if( !stats )
      return;

   struct passwd* user_info = getpwuid( stats->uid );
   struct group* group_info = getgrgid( stats->gid );
   printf( "   Relative Name ...... %s", stats->name );
      
      // add '/' if it is a dir
   if( S_ISDIR( stats->mode ) )
      printf( "/\n" );
   else
      printf( "\n" );

   printf( "   Inode Number ....... %ld\n", stats->inode );

      // add (self) if gid or uid is the currently 
      // running user/group
   if( stats->uid == getuid() )
      printf( "   Owner .............. %s (self)\n", user_info->pw_name );
   else
      printf( "   Owner .............. %s\n", user_info->pw_name );

   if( stats->gid == getgid() )
      printf( "   Group .............. %s (self)\n", group_info->gr_name );
   else
      printf( "   Group .............. %s\n", group_info->gr_name );

   printf( "   File Size (bytes) .. %ld\n", stats->size );
   printf( "   Last Modified ...... %s", ctime( &(stats->mtime) ) );
   
   char perm[12];
   strmode( stats->mode, perm );
   printf( "   Access Rights ...... %s\n", perm );
   printf( "\n" );

}

   // compare two compiled_stats_t (for qsort)
   // returns based on the name of the file
int stat_cmp( const void *elem1, const void *elem2 )
{
   compiled_stats_t *stats1 = (compiled_stats_t*) elem1;
   compiled_stats_t *stats2 = (compiled_stats_t*) elem2;

   return strcmp( stats1->name, stats2->name );
}
