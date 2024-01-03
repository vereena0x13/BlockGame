bool dir_exists(cstr s) {
    auto dir = opendir(s);
    if(dir) {
        closedir(dir);
        return true;
    }
    return false;
}

bool dir_exists(rstr s) {
    return dir_exists(cast(cstr, s));
}

bool mkdir_if_not_exists(cstr s, u32 mode = 0777) {
    if(!dir_exists(s)) {
        assert(mkdir(s, mode) == 0);
        log(TRACE, "Created directory '%s'", s);
        return true;
    }
    return false;
}

bool mkdir_if_not_exists(rstr s, u32 mode = 0777) {
    return mkdir_if_not_exists(cast(cstr, s), mode);
}

bool copy_file(cstr src, cstr dst) {
    FILE *src_fh = fopen(src, "r");
    if(!src_fh) return false;
    
    FILE *dst_fh = fopen(dst, "w");
    if(!dst_fh) {
        fclose(src_fh);
        return false;
    }

    for(u8 c = fgetc(src_fh); c != EOF && !feof(src_fh); c = fgetc(src_fh)) {
        fputc(c, dst_fh);
    }

    fclose(src_fh);
    fclose(dst_fh);
 
    return true;
}

bool copy_file(rstr src, rstr dst) {
    return copy_file(cast(cstr, src), cast(cstr, dst));
}

bool delete_file(cstr path) {
    return remove(path) == 0;
}

bool delete_file(rstr path) { return delete_file(cast(cstr, path)); }

bool file_exists(cstr path) {
    FILE *fh = fopen(path, "r");
    if(fh) {
        fclose(fh);
        return true;
    }
    return false;
}

bool file_exists(rstr path) { return file_exists(cast(cstr, path)); }