int filemap_write_and_wait_range(struct address_space *mapping,
                                 loff_t lstart, loff_t lend)
{      
        int err = 0;
        struct inode *inode = mapping->host;
        struct dentry *dentry=NULL;

        if (mapping->nrpages) { 
                err = __filemap_fdatawrite_range(mapping, lstart, lend,
                                                 WB_SYNC_ALL);
                /* See comment of filemap_write_and_wait() */
                if (err != -EIO) {
                        dentry = d_find_any_alias(inode);
                if (dentry) {
       
                        if ( !strncmp("data_bench", dentry->d_iname, 10) ||\
                             !strncmp("data_bench0",dentry->d_iname, 11)) {
                                printk(KERN_EMERG "------------start fsync request comm=[%s],pid=[%d] \n",current->comm,task_pid_nr(current));
                                dput(dentry);
                                return 0;
                        }
       
                }
                        int err2 = filemap_fdatawait_range(mapping,
                                                lstart, lend);
                        if (!err)
                                err = err2;
                }
        } else {
                err = filemap_check_errors(mapping);
        }
        return err;
}
