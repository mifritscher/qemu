
/*
 * Virtio 9p backend
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Aneesh Kumar K.V <aneesh.kumar@linux.vnet.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#include "qemu/osdep.h"
#include "fsdev/qemu-fsdev.h"
#include "qemu/thread.h"
#include "qemu/coroutine.h"
#include "coth.h"
/* mifritscher: after killing the debug printf, kill this as well! */
#include "qemu/error-report.h"

int v9fs_co_readdir_r(V9fsPDU *pdu, V9fsFidState *fidp, struct dirent *dent,
                      struct dirent **result)
{
    int err;
    V9fsState *s = pdu->s;

    if (v9fs_request_cancelled(pdu)) {
        return -EINTR;
    }
    v9fs_co_run_in_worker(
        {
            errno = 0;
            err = s->ops->readdir_r(&s->ctx, &fidp->fs, dent, result);
            if (!*result && errno) {
                err = -errno;
            } else {
                err = 0;
            }
        });
#ifdef WIN32
    error_printf("v9fs_co_readdir_r: %s %d %p\n", (&fidp->fs)->dir->dd_name, err, *result);
#else
    error_printf("v9fs_co_readdir_r: %d %p\n", err, *result);
#endif
    return err;
}

off_t v9fs_co_telldir(V9fsPDU *pdu, V9fsFidState *fidp)
{
    off_t err;
    V9fsState *s = pdu->s;

    if (v9fs_request_cancelled(pdu)) {
        return -EINTR;
    }
    v9fs_co_run_in_worker(
        {
            err = s->ops->telldir(&s->ctx, &fidp->fs);
            if (err < 0) {
                err = -errno;
            }
        });
#ifdef WIN32
    error_printf("v9fs_co_telldir_r: %s %lld\n", (&fidp->fs)->dir->dd_name, err);
#else
    error_printf("v9fs_co_telldir_r: %ld\n", err);
#endif
    return err;
}

void v9fs_co_seekdir(V9fsPDU *pdu, V9fsFidState *fidp, off_t offset)
{
    V9fsState *s = pdu->s;
    if (v9fs_request_cancelled(pdu)) {
        return;
    }
    v9fs_co_run_in_worker(
        {
            s->ops->seekdir(&s->ctx, &fidp->fs, offset);
        });
#ifdef WIN32
    error_printf("v9fs_co_seekdir: %s\n", (&fidp->fs)->dir->dd_name);
#else
    error_printf("v9fs_co_seekdir\n");
#endif
}

void v9fs_co_rewinddir(V9fsPDU *pdu, V9fsFidState *fidp)
{
    V9fsState *s = pdu->s;
    if (v9fs_request_cancelled(pdu)) {
        return;
    }
    v9fs_co_run_in_worker(
        {
            s->ops->rewinddir(&s->ctx, &fidp->fs);
        });
#ifdef WIN32
    error_printf("v9fs_co_rewinddir: %s\n", (&fidp->fs)->dir->dd_name);
#else
    error_printf("v9fs_co_rewinddir\n");
#endif
}

int v9fs_co_mkdir(V9fsPDU *pdu, V9fsFidState *fidp, V9fsString *name,
                  mode_t mode, uid_t uid, gid_t gid, struct stat *stbuf)
{
    int err;
    FsCred cred;
    V9fsPath path;
    V9fsState *s = pdu->s;

    if (v9fs_request_cancelled(pdu)) {
        return -EINTR;
    }
    cred_init(&cred);
    cred.fc_mode = mode;
    cred.fc_uid = uid;
    cred.fc_gid = gid;
    v9fs_path_read_lock(s);
    v9fs_co_run_in_worker(
        {
            err = s->ops->mkdir(&s->ctx, &fidp->path, name->data,  &cred);
            if (err < 0) {
                err = -errno;
            } else {
                v9fs_path_init(&path);
                err = v9fs_name_to_path(s, &fidp->path, name->data, &path);
                if (!err) {
                    err = s->ops->lstat(&s->ctx, &path, stbuf);
                    if (err < 0) {
                        err = -errno;
                    }
                }
                v9fs_path_free(&path);
            }
        });
    v9fs_path_unlock(s);
#ifdef WIN32
    error_printf("v9fs_co_mkdir: %s %d\n", (&fidp->fs)->dir->dd_name, err);
#else
    error_printf("v9fs_co_mkdir: %d\n", err);
#endif
    return err;
}

int v9fs_co_opendir(V9fsPDU *pdu, V9fsFidState *fidp)
{
    int err;
    V9fsState *s = pdu->s;

    if (v9fs_request_cancelled(pdu)) {
        return -EINTR;
    }
    v9fs_path_read_lock(s);
    v9fs_co_run_in_worker(
        {
            err = s->ops->opendir(&s->ctx, &fidp->path, &fidp->fs);
            if (err < 0) {
                err = -errno;
            } else {
                err = 0;
            }
        });
    v9fs_path_unlock(s);
    if (!err) {
        total_open_fd++;
        if (total_open_fd > open_fd_hw) {
            v9fs_reclaim_fd(pdu);
        }
    }
#ifdef WIN32
    error_printf("v9fs_co_opendir: %s %d\n", (&fidp->fs)->dir->dd_name, err);
#else
    error_printf("v9fs_co_opendir: %d\n", err);
#endif
    return err;
}

int v9fs_co_closedir(V9fsPDU *pdu, V9fsFidOpenState *fs)
{
    int err;
    V9fsState *s = pdu->s;

    if (v9fs_request_cancelled(pdu)) {
        return -EINTR;
    }
    v9fs_co_run_in_worker(
        {
            err = s->ops->closedir(&s->ctx, fs);
            if (err < 0) {
                err = -errno;
            }
        });
    if (!err) {
        total_open_fd--;
    }
    error_printf("v9fs_co_closedir: %d\n", err);
    return err;
}
