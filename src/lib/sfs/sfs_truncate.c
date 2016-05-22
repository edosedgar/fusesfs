#include <bdev/blockdev.h>
#include <sfs/defs.h>
#include <sfs/debug.h>
#include <sfs/unit.h>
#include <sfs/callback.h>
#include <sfs/fsutils.h>
#include <sfs/utils.h>
#include <sfs/alloc.h>

#define AS_FILE(entr) ((file_entry*) (entr))
#define AS_DFILE(entr) ((del_file_entry*) (entr))

int sfs_truncate(sfs_unit* fs, off_t file, size_t new_size)
{
        entry entr;
        size_t old_size = 0;
        off_t old_start = 0;
        off_t old_end = 0;
        off_t new_start = 0;
        off_t new_end = 0;

        if (read_entry(fs->bdev, file, &entr) == -1) {
                SET_ERRNO(EIO);
                return 0;
        }

        old_size = AS_FILE(&entr)->size;
        old_start = AS_FILE(&entr)->start_block;
        old_end = AS_FILE(&entr)->end_block;
        SFS_TRACE("Try truncate file:\n"
                        "Size:  %lu\n"
                        "Start: %lu\n"
                        "End:   %lu"
                        "New size : %lu\n", 
                        old_size, old_start, 
                        old_end, new_size);
 
        new_start = old_start;
        new_end = old_end;

        if (new_size == 0) {
                del_file_list_add(fs, &entr, old_start, old_end);
                new_start = 0;
                new_end = 0;
                goto END;
        }

        if (get_real_size(fs, new_size) < get_real_size(fs, old_size)) {
                if (file_shrink(fs, file, new_size, &entr) != 0) {
                        return -1;
                }
                new_end = new_start + 
                          (get_real_size(fs, new_size) 
                               / fs->bdev->block_size) - 1;
                goto END;
        }

        if (get_real_size(fs, new_size) > get_real_size(fs, old_size)) {
                if (new_start != 0) {
                        if (try_expand(fs, file, new_size, &entr) == 0) {
                                new_end = new_start + 
                                          (get_real_size(fs, new_size) 
                                               / fs->bdev->block_size) - 1;
                                goto END;
                        }
                } 
                if ((new_start = del_file_list_alloc(fs, &entr, new_size))
                                        == 0) {
                        SFS_TRACE("Can't alloc space");
                        return -1;
                }
                new_end = new_start + (get_real_size(fs, new_size) 
                                       / fs->bdev->block_size) - 1;
                
                if (old_start != 0) {
                        del_file_list_add(fs, &entr, old_start, old_end);
                        if (copy_block(fs->bdev, old_start, new_start, 
                                       old_end - old_start + 1) != 0) {
                                SFS_TRACE("Can't copy blocks");
                                return -1;
                        }
                }
        }
END:
        read_entry(fs->bdev, file, &entr);
        AS_FILE(&entr)->start_block = new_start;
        AS_FILE(&entr)->end_block = new_end;
        AS_FILE(&entr)->size = new_size;
        AS_FILE(&entr)->time_stamp = get_time();
        write_entry(fs->bdev, file, &entr);
 
        update(fs);
        return 0;
}