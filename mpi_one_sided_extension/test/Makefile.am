#
# Copyright 2014-2016, Gdansk University of Technology
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holder nor the names of its contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#


LOG_COMPILER = $(srcdir)/mpiexec-test

TESTS = parse_mpi_info_bool_true.1 parse_mpi_info_bool_false.1 parse_mpi_info_bool_not_set.1 parse_mpi_info_bool_wrong_value.1 \
        parse_mpi_info_name_valid.1 parse_mpi_info_name_too_long.1 parse_mpi_info_name_not_set.1 \
        parse_mpi_info_mode_expand.1 parse_mpi_info_mode_checkpoint.1 parse_mpi_info_mode_wrong_value.1 parse_mpi_info_mode_not_set.1 \
        parse_mpi_info_checkpoint_version_correct_value.1 parse_mpi_info_checkpoint_version_not_set.1 \
        check_if_window_exists_and_its_size_first.1 check_if_window_exists_and_its_size_middle.1 check_if_window_exists_and_its_size_last.1 \
        check_if_window_exists_and_its_size_deleted_first.1 check_if_window_exists_and_its_size_deleted_middle.1 check_if_window_exists_and_its_size_deleted_last.1 \
        check_if_window_exists_and_its_size_non_existing.1 check_if_window_exists_and_its_size_correct_size.1 check_if_window_exists_and_its_size_incorrect_size.1 \
        check_if_window_exists_and_its_size_incorrect_size_non_existing.1 \
        create_window_metadata_file_new_windows.1 create_window_metadata_file_recreate_first.1 create_window_metadata_file_recreate_middle.1 create_window_metadata_file_recreate_last.1 \
        update_window_size_in_metadata_file_first.1 update_window_size_in_metadata_file_middle.1 update_window_size_in_metadata_file_last.1 \
        update_window_size_in_metadata_file_non_existing.1 update_window_size_in_metadata_file_deleted.1 \
        set_checkpoint_versions_expand.1 set_checkpoint_versions_last.1 set_checkpoint_versions_last_not_highest.1 set_checkpoint_versions_last_2_processes.2 \
        set_checkpoint_versions_last_2_processes_no_checkpoint.2 set_checkpoint_versions_correct.1 set_checkpoint_versions_correct_append.1 set_checkpoint_versions_negative.1 \
        set_checkpoint_versions_too_high.1 set_checkpoint_versions_deleted.1 \
        copy_data_from_checkpoint_existing.1 copy_data_from_checkpoint_non_existing.1 copy_data_from_checkpoint_deleted.1 \
        delete_old_checkpoints_all.1 delete_old_checkpoints_first.1 delete_old_checkpoints_middle.1 delete_old_checkpoints_last.1 \
        MPI_Win_create_pmem_is_pmem.1 MPI_Win_create_pmem_empty_info.1 MPI_Win_create_pmem_info_null.1 \
        MPI_Win_allocate_pmem_expand.1 MPI_Win_allocate_pmem_checkpoint_non_existing.1 MPI_Win_allocate_pmem_expand_existing.1 MPI_Win_allocate_pmem_checkpoint.1 \
        MPI_Win_create_dynamic_pmem_is_pmem.1 MPI_Win_create_dynamic_pmem_empty_info.1 MPI_Win_create_dynamic_pmem_info_null.1 \
        MPI_Win_attach_pmem.1 \
        MPI_Win_detach_pmem_first.1 MPI_Win_detach_pmem_middle.1 MPI_Win_detach_pmem_last.1 \
        MPI_Win_set_info_pmem_create.1 MPI_Win_set_info_pmem_allocate.1 \
        create_checkpoint_consecutive_keep_all.1 create_checkpoint_overwrite_keep_all.1 create_checkpoint_append_keep_all.1 \
        create_checkpoint_consecutive_dont_keep_all.1 create_checkpoint_overwrite_dont_keep_all.1 create_checkpoint_append_dont_keep_all.1 \
        MPI_Win_pmem_set_root_path_too_long.1 MPI_Win_pmem_set_root_path_non_existing.1 MPI_Win_pmem_set_root_path_regular_file.1 \
        MPI_Win_pmem_set_root_path_new.1 MPI_Win_pmem_set_root_path_reload.1 \
        MPI_Win_pmem_list.1 \
        MPI_Win_pmem_get_versions.1 \
        MPI_Win_pmem_delete_all.1 MPI_Win_pmem_delete_deleted.1 MPI_Win_pmem_delete_non_existing.1 \
        MPI_Win_pmem_delete_no_first.1 MPI_Win_pmem_delete_no_middle.1 MPI_Win_pmem_delete_no_last.1 \
        MPI_Win_pmem_delete_version_non_existing_window.1 MPI_Win_pmem_delete_version_deleted_window.1 MPI_Win_pmem_delete_version_non_existing_version.1 \
        MPI_Win_pmem_delete_version_deleted_version.1 MPI_Win_pmem_delete_version_first.1 MPI_Win_pmem_delete_version_middle.1 MPI_Win_pmem_delete_version_last.1

check_PROGRAMS = parse_mpi_info_bool_true.1 parse_mpi_info_bool_false.1 parse_mpi_info_bool_not_set.1 parse_mpi_info_bool_wrong_value.1 \
                 parse_mpi_info_name_valid.1 parse_mpi_info_name_too_long.1 parse_mpi_info_name_not_set.1 \
                 parse_mpi_info_mode_expand.1 parse_mpi_info_mode_checkpoint.1 parse_mpi_info_mode_wrong_value.1 parse_mpi_info_mode_not_set.1 \
                 parse_mpi_info_checkpoint_version_correct_value.1 parse_mpi_info_checkpoint_version_not_set.1 \
                 check_if_window_exists_and_its_size_first.1 check_if_window_exists_and_its_size_middle.1 check_if_window_exists_and_its_size_last.1 \
                 check_if_window_exists_and_its_size_deleted_first.1 check_if_window_exists_and_its_size_deleted_middle.1 check_if_window_exists_and_its_size_deleted_last.1 \
                 check_if_window_exists_and_its_size_non_existing.1 check_if_window_exists_and_its_size_correct_size.1 check_if_window_exists_and_its_size_incorrect_size.1 \
                 check_if_window_exists_and_its_size_incorrect_size_non_existing.1 \
                 create_window_metadata_file_new_windows.1 create_window_metadata_file_recreate_first.1 create_window_metadata_file_recreate_middle.1 create_window_metadata_file_recreate_last.1 \
                 update_window_size_in_metadata_file_first.1 update_window_size_in_metadata_file_middle.1 update_window_size_in_metadata_file_last.1 \
                 update_window_size_in_metadata_file_non_existing.1 update_window_size_in_metadata_file_deleted.1 \
                 set_checkpoint_versions_expand.1 set_checkpoint_versions_last.1 set_checkpoint_versions_last_not_highest.1 set_checkpoint_versions_last_2_processes.2 \
                 set_checkpoint_versions_last_2_processes_no_checkpoint.2 set_checkpoint_versions_correct.1 set_checkpoint_versions_correct_append.1 set_checkpoint_versions_negative.1 \
                 set_checkpoint_versions_too_high.1 set_checkpoint_versions_deleted.1 \
                 copy_data_from_checkpoint_existing.1 copy_data_from_checkpoint_non_existing.1 copy_data_from_checkpoint_deleted.1 \
                 delete_old_checkpoints_all.1 delete_old_checkpoints_first.1 delete_old_checkpoints_middle.1 delete_old_checkpoints_last.1 \
                 MPI_Win_create_pmem_is_pmem.1 MPI_Win_create_pmem_empty_info.1 MPI_Win_create_pmem_info_null.1 \
                 MPI_Win_allocate_pmem_expand.1 MPI_Win_allocate_pmem_checkpoint_non_existing.1 MPI_Win_allocate_pmem_expand_existing.1 MPI_Win_allocate_pmem_checkpoint.1 \
                 MPI_Win_create_dynamic_pmem_is_pmem.1 MPI_Win_create_dynamic_pmem_empty_info.1 MPI_Win_create_dynamic_pmem_info_null.1 \
                 MPI_Win_attach_pmem.1 \
                 MPI_Win_detach_pmem_first.1 MPI_Win_detach_pmem_middle.1 MPI_Win_detach_pmem_last.1 \
                 MPI_Win_set_info_pmem_create.1 MPI_Win_set_info_pmem_allocate.1 \
                 create_checkpoint_consecutive_keep_all.1 create_checkpoint_overwrite_keep_all.1 create_checkpoint_append_keep_all.1 \
                 create_checkpoint_consecutive_dont_keep_all.1 create_checkpoint_overwrite_dont_keep_all.1 create_checkpoint_append_dont_keep_all.1 \
                 MPI_Win_pmem_set_root_path_too_long.1 MPI_Win_pmem_set_root_path_non_existing.1 MPI_Win_pmem_set_root_path_regular_file.1 \
                 MPI_Win_pmem_set_root_path_new.1 MPI_Win_pmem_set_root_path_reload.1 \
                 MPI_Win_pmem_list.1 \
                 MPI_Win_pmem_get_versions.1 \
                 MPI_Win_pmem_delete_all.1 MPI_Win_pmem_delete_deleted.1 MPI_Win_pmem_delete_non_existing.1 \
                 MPI_Win_pmem_delete_no_first.1 MPI_Win_pmem_delete_no_middle.1 MPI_Win_pmem_delete_no_last.1 \
                 MPI_Win_pmem_delete_version_non_existing_window.1 MPI_Win_pmem_delete_version_deleted_window.1 MPI_Win_pmem_delete_version_non_existing_version.1 \
                 MPI_Win_pmem_delete_version_deleted_version.1 MPI_Win_pmem_delete_version_first.1 MPI_Win_pmem_delete_version_middle.1 MPI_Win_pmem_delete_version_last.1

AM_CFLAGS +=
AM_CPPFLAGS += -I$(srcdir)/../src
LDADD += ../src/mpi_one_sided_extension/libmpi-pmem-one-sided.la
AM_LDFLAGS +=

parse_mpi_info_bool_true_1_SOURCES = parse_mpi_info_bool_true.c
parse_mpi_info_bool_false_1_SOURCES = parse_mpi_info_bool_false.c
parse_mpi_info_bool_not_set_1_SOURCES = parse_mpi_info_bool_not_set.c
parse_mpi_info_bool_wrong_value_1_SOURCES = parse_mpi_info_bool_wrong_value.c

parse_mpi_info_name_valid_1_SOURCES = parse_mpi_info_name_valid.c
parse_mpi_info_name_too_long_1_SOURCES = parse_mpi_info_name_too_long.c
parse_mpi_info_name_not_set_1_SOURCES = parse_mpi_info_name_not_set.c

parse_mpi_info_mode_expand_1_SOURCES = parse_mpi_info_mode_expand.c
parse_mpi_info_mode_checkpoint_1_SOURCES = parse_mpi_info_mode_checkpoint.c
parse_mpi_info_mode_wrong_value_1_SOURCES = parse_mpi_info_mode_wrong_value.c
parse_mpi_info_mode_not_set_1_SOURCES = parse_mpi_info_mode_not_set.c

parse_mpi_info_checkpoint_version_correct_value_1_SOURCES = parse_mpi_info_checkpoint_version_correct_value.c
parse_mpi_info_checkpoint_version_not_set_1_SOURCES = parse_mpi_info_checkpoint_version_not_set.c

check_if_window_exists_and_its_size_first_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_first.c
check_if_window_exists_and_its_size_middle_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_middle.c
check_if_window_exists_and_its_size_last_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_last.c
check_if_window_exists_and_its_size_deleted_first_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_deleted_first.c
check_if_window_exists_and_its_size_deleted_middle_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_deleted_middle.c
check_if_window_exists_and_its_size_deleted_last_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_deleted_last.c
check_if_window_exists_and_its_size_non_existing_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_non_existing.c
check_if_window_exists_and_its_size_correct_size_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_correct_size.c
check_if_window_exists_and_its_size_incorrect_size_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_incorrect_size.c
check_if_window_exists_and_its_size_incorrect_size_non_existing_1_SOURCES = helper.c helper.h check_if_window_exists_and_its_size_incorrect_size_non_existing.c

create_window_metadata_file_new_windows_1_SOURCES = helper.c helper.h create_window_metadata_file_new_windows.c
create_window_metadata_file_recreate_first_1_SOURCES = helper.c helper.h create_window_metadata_file_recreate_first.c
create_window_metadata_file_recreate_middle_1_SOURCES = helper.c helper.h create_window_metadata_file_recreate_middle.c
create_window_metadata_file_recreate_last_1_SOURCES = helper.c helper.h create_window_metadata_file_recreate_last.c

update_window_size_in_metadata_file_first_1_SOURCES = helper.c helper.h update_window_size_in_metadata_file_first.c
update_window_size_in_metadata_file_middle_1_SOURCES = helper.c helper.h update_window_size_in_metadata_file_middle.c
update_window_size_in_metadata_file_last_1_SOURCES = helper.c helper.h update_window_size_in_metadata_file_last.c
update_window_size_in_metadata_file_non_existing_1_SOURCES = helper.c helper.h update_window_size_in_metadata_file_non_existing.c
update_window_size_in_metadata_file_deleted_1_SOURCES = helper.c helper.h update_window_size_in_metadata_file_deleted.c

set_checkpoint_versions_expand_1_SOURCES = helper.c helper.h set_checkpoint_versions_expand.c
set_checkpoint_versions_last_1_SOURCES = helper.c helper.h set_checkpoint_versions_last.c
set_checkpoint_versions_last_not_highest_1_SOURCES = helper.c helper.h set_checkpoint_versions_last_not_highest.c
set_checkpoint_versions_last_2_processes_2_SOURCES = helper.c helper.h set_checkpoint_versions_last_2_processes.c
set_checkpoint_versions_last_2_processes_no_checkpoint_2_SOURCES = helper.c helper.h set_checkpoint_versions_last_2_processes_no_checkpoint.c
set_checkpoint_versions_correct_1_SOURCES = helper.c helper.h set_checkpoint_versions_correct.c
set_checkpoint_versions_correct_append_1_SOURCES = helper.c helper.h set_checkpoint_versions_correct_append.c
set_checkpoint_versions_negative_1_SOURCES = helper.c helper.h set_checkpoint_versions_negative.c
set_checkpoint_versions_too_high_1_SOURCES = helper.c helper.h set_checkpoint_versions_too_high.c
set_checkpoint_versions_deleted_1_SOURCES = helper.c helper.h set_checkpoint_versions_deleted.c

copy_data_from_checkpoint_existing_1_SOURCES = helper.c helper.h copy_data_from_checkpoint_existing.c
copy_data_from_checkpoint_non_existing_1_SOURCES = helper.c helper.h copy_data_from_checkpoint_non_existing.c
copy_data_from_checkpoint_deleted_1_SOURCES = helper.c helper.h copy_data_from_checkpoint_deleted.c

delete_old_checkpoints_all_1_SOURCES = helper.c helper.h delete_old_checkpoints_all.c
delete_old_checkpoints_first_1_SOURCES = helper.c helper.h delete_old_checkpoints_first.c
delete_old_checkpoints_middle_1_SOURCES = helper.c helper.h delete_old_checkpoints_middle.c
delete_old_checkpoints_last_1_SOURCES = helper.c helper.h delete_old_checkpoints_last.c

MPI_Win_create_pmem_is_pmem_1_SOURCES = helper.c helper.h MPI_Win_create_pmem_is_pmem.c
MPI_Win_create_pmem_empty_info_1_SOURCES = helper.c helper.h MPI_Win_create_pmem_empty_info.c
MPI_Win_create_pmem_info_null_1_SOURCES = helper.c helper.h MPI_Win_create_pmem_info_null.c

MPI_Win_allocate_pmem_expand_1_SOURCES = helper.c helper.h MPI_Win_allocate_pmem_expand.c
MPI_Win_allocate_pmem_checkpoint_non_existing_1_SOURCES = MPI_Win_allocate_pmem_checkpoint_non_existing.c
MPI_Win_allocate_pmem_expand_existing_1_SOURCES = helper.c helper.h MPI_Win_allocate_pmem_expand_existing.c
MPI_Win_allocate_pmem_checkpoint_1_SOURCES = helper.c helper.h MPI_Win_allocate_pmem_checkpoint.c

MPI_Win_create_dynamic_pmem_is_pmem_1_SOURCES = helper.c helper.h MPI_Win_create_dynamic_pmem_is_pmem.c
MPI_Win_create_dynamic_pmem_empty_info_1_SOURCES = helper.c helper.h MPI_Win_create_dynamic_pmem_empty_info.c
MPI_Win_create_dynamic_pmem_info_null_1_SOURCES = helper.c helper.h MPI_Win_create_dynamic_pmem_info_null.c

MPI_Win_attach_pmem_1_SOURCES = helper.c helper.h MPI_Win_attach_pmem.c

MPI_Win_detach_pmem_first_1_SOURCES = helper.c helper.h MPI_Win_detach_pmem_first.c
MPI_Win_detach_pmem_middle_1_SOURCES = helper.c helper.h MPI_Win_detach_pmem_middle.c
MPI_Win_detach_pmem_last_1_SOURCES = helper.c helper.h MPI_Win_detach_pmem_last.c

MPI_Win_set_info_pmem_create_1_SOURCES = helper.c helper.h MPI_Win_set_info_pmem_create.c
MPI_Win_set_info_pmem_allocate_1_SOURCES = helper.c helper.h MPI_Win_set_info_pmem_allocate.c

create_checkpoint_consecutive_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_consecutive_keep_all.c
create_checkpoint_overwrite_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_overwrite_keep_all.c
create_checkpoint_append_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_append_keep_all.c
create_checkpoint_consecutive_dont_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_consecutive_dont_keep_all.c
create_checkpoint_overwrite_dont_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_overwrite_dont_keep_all.c
create_checkpoint_append_dont_keep_all_1_SOURCES = helper.c helper.h create_checkpoint_append_dont_keep_all.c

MPI_Win_pmem_set_root_path_too_long_1_SOURCES = MPI_Win_pmem_set_root_path_too_long.c
MPI_Win_pmem_set_root_path_non_existing_1_SOURCES = MPI_Win_pmem_set_root_path_non_existing.c
MPI_Win_pmem_set_root_path_regular_file_1_SOURCES = MPI_Win_pmem_set_root_path_regular_file.c
MPI_Win_pmem_set_root_path_new_1_SOURCES = helper.c helper.h MPI_Win_pmem_set_root_path_new.c
MPI_Win_pmem_set_root_path_reload_1_SOURCES = helper.c helper.h MPI_Win_pmem_set_root_path_reload.c

MPI_Win_pmem_list_1_SOURCES = helper.c helper.h MPI_Win_pmem_list.c

MPI_Win_pmem_get_versions_1_SOURCES = helper.c helper.h MPI_Win_pmem_get_versions.c

MPI_Win_pmem_delete_all_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_all.c
MPI_Win_pmem_delete_deleted_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_deleted.c
MPI_Win_pmem_delete_non_existing_1_SOURCES= MPI_Win_pmem_delete_non_existing.c
MPI_Win_pmem_delete_no_first_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_no_first.c
MPI_Win_pmem_delete_no_middle_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_no_middle.c
MPI_Win_pmem_delete_no_last_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_no_last.c

MPI_Win_pmem_delete_version_non_existing_window_1_SOURCES = MPI_Win_pmem_delete_version_non_existing_window.c
MPI_Win_pmem_delete_version_deleted_window_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_deleted_window.c
MPI_Win_pmem_delete_version_non_existing_version_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_non_existing_version.c
MPI_Win_pmem_delete_version_deleted_version_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_deleted_version.c
MPI_Win_pmem_delete_version_first_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_first.c
MPI_Win_pmem_delete_version_middle_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_middle.c
MPI_Win_pmem_delete_version_last_1_SOURCES = helper.c helper.h MPI_Win_pmem_delete_version_last.c
