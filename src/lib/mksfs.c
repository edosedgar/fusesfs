#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <mksfs/mksfs.h>


int image_create(struct sfs_options sfs_opts) {
        size_t block_size = pow(2, sfs_opts.block_size + BEGIN_POWER_OF_BS);
        /*
         * Print volume info 
         */
        fprintf(stdout, "\nTime stamp: %s"
                        "Data size: %lu blocks\n"
                        "Index size: %lu bytes\n"
                        "Total blocks: %lu blocks\n"
                        "Block size: %lu bytes\n"
                        "Label: %s\n"
                        "File name: %s\n", 
                        ctime(&(sfs_opts.time_stamp)), sfs_opts.data_size,
                        sfs_opts.index_size, sfs_opts.total_block,
                        block_size, sfs_opts.label,
                        sfs_opts.file_name); 
                              

        return 0;
}