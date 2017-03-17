#include <truth/file.h>
#include <truth/heap.h>
#include <truth/string.h>
#include <truth/panic.h>


struct file Root = {
    .name = "",
    .type = File_Directory,
    .read = NULL,
    .write = NULL,
    .init = NULL,
    .fini = NULL,
    .parent = NULL,
    .child_capacity = 0,
    .child_count = 0,
    .children = NULL,
    .permissions = 0755,
    .obj.ref_count = 1,
    .obj.free = NULL,
};


struct file Dev = {
    .name = "dev",
    .type = File_Directory,
    .read = NULL,
    .write = NULL,
    .init = NULL,
    .fini = NULL,
    .parent = &Root,
    .child_capacity = 0,
    .child_count = 0,
    .children = NULL,
    .permissions = 0755,
    .obj.ref_count = 1,
    .obj.free = NULL,
};


enum status file_system_init(void) {
    enum status status =  file_attach(&Root, &Dev);
    log(Log_Info, "Virtual File System initialized");
    return status;
}


void file_fini(struct file *file) {
    assert(file != NULL);
    for (size_t i = 0; i < file->child_count; ++i) {
        file_fini(file->children[i]);
    }
    if (file->fini != NULL) {
        file->fini();
    }
    object_release(&file->obj);
}


void file_system_fini(void) {
    file_fini(&Root);
}


struct file *file_find_child_named(struct file *file, const char *name,
                                   size_t n) {
    assert(file != NULL);
    assert(name != NULL);
    for (size_t i = 0; i < file->child_count; ++i) {
        if (strncmp(name, file->children[i]->name, n) == Order_Equal) {
            object_retain(&file->children[i]->obj);
            return file->children[i];
        }
    }
    return NULL;
}


struct file *file_from_path(const char *path) {
    assert(path != NULL);
    struct file *parent = &Root;
    const char *start = path;
    for (size_t i = 0; path[i] != '\0'; ++i) {
        if (path[i] == '/') {
            parent = file_find_child_named(parent, start, start - &path[i]);
            start = &path[i+1];
            if (parent == NULL) {
                return NULL;
            }
        }
    }
    return parent;
}


static enum status file_name_is_valid(const char *path) {
    if (path[0] == '\0') {
        return Error_Invalid;
    }
    for (size_t i = 0; path[i] != '\0'; ++i) {
        if (path[i] == '/') {
            return Error_Invalid;
        }
    }
    return Ok;
}


enum status file_attach_path(const char *parent_path, struct file *child) {
    struct file *parent = file_from_path(parent_path);
    if (parent == NULL) {
        return Error_Invalid;
    }
    file_attach(parent, child);
    object_release(&parent->obj);
    return Ok;
}


enum status file_attach(struct file *parent, struct file *child) {
    assert(parent != NULL);
    assert(child != NULL);

    enum status status = file_name_is_valid(child->name);
    if (status != Ok) {
        return status;
    }

    if (parent->child_capacity == parent->child_count) {
        parent->child_capacity += 10;
        struct file **new_children = krealloc(parent->children,
                                              parent->child_capacity *
                                              sizeof(struct file *));
        if (new_children == NULL) {
            return Error_No_Memory;
        }
        parent->children = new_children;
    }

    parent->children[parent->child_count] = child;
    parent->child_count++;
    child->parent = parent;
    object_retain(&parent->obj);

    return Ok;
}
