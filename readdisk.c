#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "include/types.h"
#include "include/fs.h"
#include "include/stat.h"

void show_super_block (int fd)
{
     unsigned char buf [1024];

     // Super Block always at block 1
    lseek (fd, 1*1024, 0);

    // Read super block
    read (fd, buf, sizeof (buf));

    struct superblock sb;

    memmove (&sb, buf, sizeof (sb));

    printf ("Size File System    = %d\n", sb.size);
    printf ("Number of block     = %d\n", sb.nblocks);
    printf ("Number of inodes    = %d\n", sb.ninodes);
    printf ("Number of log block = %d\n", sb.nlog);

}

void show_directory (int fd)
{
    unsigned char buf [1024];

    struct dirent dir;

    lseek (fd, 16*1024, 0);

    while (read (fd, buf, sizeof (struct dirent)))
    {
        memmove (&dir, buf, sizeof(struct dirent));
        if (dir.inum==0)
            break;
        printf ("%d - %s\n", dir.inum, dir.name);
    }
}

void show_inode_details (int fd)
{
    unsigned char buf [1024];
    char input [128];

    struct dinode di;

    printf ("Enter inode number: ");

    while (fgets(input, sizeof (input), stdin))
    {
        if (input [0]=='q')
            return;

        int inode_number = atoi (input);

        lseek (fd, (2*1024)+(inode_number*64), 0);
        
        read (fd, buf, sizeof (struct dinode));
        memmove (&di, buf, sizeof(struct dinode));
        printf ("inode type  : %d\n", di.type);
        printf ("inode size  : %d\n", di.size);
        printf ("inode major : %d\n", di.major);
        printf ("inode minor : %d\n", di.minor);
        printf ("inode links : %d\n", di.nlink);

        printf ("Enter inode number [q=exit]: ");
    }
}

struct bitmap_header {
    
    // header
    char signature      [2];
    char filesize       [4];
    char reserved       [4];
    char data_offset    [4];

} bitmap_header;

struct bitmap_info_header {
    
    // info header
    char size               [4];
    char width              [4];
    char height             [4];
    char planes             [2];
    char bit_count          [2];
    char compression        [4];
    char image_size         [4];
    char x_pixels_per_m     [4];
    char y_pixels_per_m     [4];
    char colors_used        [4];
    char colors_important   [4];
    char colors_table       [4*16];

} bitmap_info_header;

struct chunk_header {
    int length;
    char type [4];
} chunk_header;

struct __attribute__((__packed__)) chunk_ihdr {
    int width;
    int height;
    char depth;
    char color;
    char compression;
    char filter;
    char interlaced;
} chunk_ihdr;

uint32_t swap_endian(uint32_t val) {
    return ((val >> 24) & 0x000000FF) |
           ((val >> 8)  & 0x0000FF00) |
           ((val << 8)  & 0x00FF0000) |
           ((val << 24) & 0xFF000000);
}


int read_bitmap ()
{
      int fd = -1;

    unsigned char buf [1024];
    struct chunk_header header;
    struct chunk_ihdr ihdr;
//    struct bitmap_info_header info_header;

    fd = open("test.png", O_RDONLY, 0666);
    if(fd < 0) {
        perror("test.png");
        return(1);
    }

    // Skip header
    read (fd, buf, 8);

int i = sizeof (struct chunk_header);

memset (buf, 0, sizeof (buf));

    // Read chunks
    read (fd, buf, sizeof (struct chunk_header));
    memmove (&header, buf, sizeof(struct chunk_header));

    header.length = swap_endian (header.length);

if (strcmp (header.type, "IHDR")) {

int k = sizeof (struct chunk_ihdr);

     read (fd, buf, sizeof (struct chunk_ihdr));
     memmove (&ihdr, buf, sizeof(struct chunk_ihdr));

    ihdr.width = swap_endian (ihdr.width);

read (fd, buf, 4); // CRC

     printf ("width = %d\n", ihdr.width);
}


memset (buf, 0, sizeof (buf));

   // Read chunks
    read (fd, buf, sizeof (struct chunk_header));
    memmove (&header, buf, sizeof(struct chunk_header));

header.length = swap_endian (header.length);

if (strcmp (header.type, "sRGB")) {

read (fd, buf, 1); // Rendering

read (fd, buf, 4); // CRC
}



memset (buf, 0, sizeof (buf));

   // Read chunks
    read (fd, buf, sizeof (struct chunk_header));
    memmove (&header, buf, sizeof(struct chunk_header));

header.length = swap_endian (header.length);

    // read (fd, buf, sizeof (struct bitmap_header));
    // memmove (&header, buf, sizeof(struct bitmap_header));

    // read (fd, buf, sizeof (struct bitmap_info_header));
    // memmove (&info_header, buf, sizeof(struct bitmap_info_header));

    //printf ("Width = %d\n", info_header.width);
memset (buf, 0, sizeof (buf));
char idta [header.length];
read (fd, buf, header.length);

}












// int main()
// {
//     int fd = -1;
//     char input [128];
//     unsigned char buf [1024];

// read_bitmap ();
// return 0;

//     fd = open("fs.img", O_RDONLY, 0666);
//     if(fd < 0) {
//         perror("fs.img");
//         return(1);
//     }

//     printf ("\n");
//     printf ("xv6 disk system tool 1.0.0\n");
//     printf ("\n\n");
//     printf ("q Exit\n");
//     printf ("1 Super Block Information\n");
//     printf ("2 Show Directory\n");
//     printf ("3 Show inode details\n");
//     printf ("4 Read bitmap\n");
//     printf ("\n");
    
//     printf ("Enter menu option: ");

//     while (fgets(input, sizeof (input), stdin))
//     {
//         if (input[0]=='q')
//             return (1);

//         if (input[0]=='1')
//             show_super_block (fd);

//         if (input[0]=='2')
//             show_directory (fd);

//         if (input[0]=='3')
//             show_inode_details (fd);
        
//         printf ("Enter menu option: ");
//     }
   

//     // Read boot block
//     read (fd, buf, sizeof (buf));

//     // Read super block
//     read (fd, buf, sizeof (buf));

//     struct superblock sb;

//     memmove (&sb, buf, sizeof (sb));

//     printf ("Size File System    = %d\n", sb.size);
//     printf ("Number of block     = %d\n", sb.nblocks);
//     printf ("Number of inodes    = %d\n", sb.ninodes);
//     printf ("Number of log block = %d\n", sb.nlog);

// //    for (int i=0; i<sb.nlog + 1; i++)
// //        read (fd, buf, sizeof (buf));

//     // Read inodes (first is empty)
//     read (fd, buf, sizeof (struct dinode));

//     struct dinode di;
//     struct dirent dir;

//     read (fd, buf, sizeof (struct dinode));
//     memmove (&di, buf, sizeof(struct dinode));

//     // if (di.type==T_DIR)
//     // {
//     //     lseek (fd, 16*1024, 0);
//     //     while (read (fd, buf, sizeof (struct dirent)))
//     //     {
//     //         memmove (&dir, buf, sizeof(struct dirent));
//     //         if (dir.inum==0)
//     //             break;
//     //         printf ("%s\n", dir.name);
//     //     }
        
//     // }

   

//     read (fd, buf, sizeof (struct dinode));
//     memmove (&di, buf, sizeof(struct dinode));

//     read (fd, buf, sizeof (struct dinode));
//     memmove (&di, buf, sizeof(struct dinode));

//     read (fd, buf, sizeof (struct dinode));
//     memmove (&di, buf, sizeof(struct dinode));

//     read (fd, buf, sizeof (struct dinode));
//     memmove (&di, buf, sizeof(struct dinode));

//     // for (int i=0; i<sizeof (buf); i++)
//     // printf ("%c", buf [i]);


//     close(fd);

    

// }
