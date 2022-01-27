/*****************************************************************************

Copyright (c) 1996, 2017, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2013, 2019, MariaDB Corporation.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

*****************************************************************************/

/******************************************************************//**
@file include/dict0dict.ic
Data dictionary system

Created 1/8/1996 Heikki Tuuri
***********************************************************************/

#include "fsp0sysspace.h"

/*********************************************************************//**
Gets the minimum number of bytes per character.
@return minimum multi-byte char size, in bytes */
UNIV_INLINE
ulint
dict_col_get_mbminlen(
/*==================*/
	const dict_col_t*	col)	/*!< in: column */
{
	return col->mbminlen;
}
/*********************************************************************//**
Gets the maximum number of bytes per character.
@return maximum multi-byte char size, in bytes */
UNIV_INLINE
ulint
dict_col_get_mbmaxlen(
/*==================*/
	const dict_col_t*	col)	/*!< in: column */
{
	return col->mbmaxlen;
}
/*********************************************************************//**
Gets the column data type. */
UNIV_INLINE
void
dict_col_copy_type(
/*===============*/
	const dict_col_t*	col,	/*!< in: column */
	dtype_t*		type)	/*!< out: data type */
{
	ut_ad(col != NULL);
	ut_ad(type != NULL);

	type->mtype = col->mtype;
	type->prtype = col->prtype;
	type->len = col->len;
	type->mbminlen = col->mbminlen;
	type->mbmaxlen = col->mbmaxlen;
}

#ifdef UNIV_DEBUG
/*********************************************************************//**
Assert that a column and a data type match.
@return TRUE */
UNIV_INLINE
ibool
dict_col_type_assert_equal(
/*=======================*/
	const dict_col_t*	col,	/*!< in: column */
	const dtype_t*		type)	/*!< in: data type */
{
	ut_ad(col->mtype == type->mtype);
	ut_ad(col->prtype == type->prtype);
	//ut_ad(col->len == type->len);
	ut_ad(col->mbminlen == type->mbminlen);
	ut_ad(col->mbmaxlen == type->mbmaxlen);

	return(TRUE);
}
#endif /* UNIV_DEBUG */

/***********************************************************************//**
Returns the minimum size of the column.
@return minimum size */
UNIV_INLINE
ulint
dict_col_get_min_size(
/*==================*/
	const dict_col_t*	col)	/*!< in: column */
{
	return(dtype_get_min_size_low(col->mtype, col->prtype, col->len,
				      col->mbminlen, col->mbmaxlen));
}
/***********************************************************************//**
Returns the maximum size of the column.
@return maximum size */
UNIV_INLINE
ulint
dict_col_get_max_size(
/*==================*/
	const dict_col_t*	col)	/*!< in: column */
{
	return(dtype_get_max_size_low(col->mtype, col->len));
}
/***********************************************************************//**
Returns the size of a fixed size column, 0 if not a fixed size column.
@return fixed size, or 0 */
UNIV_INLINE
ulint
dict_col_get_fixed_size(
/*====================*/
	const dict_col_t*	col,	/*!< in: column */
	ulint			comp)	/*!< in: nonzero=ROW_FORMAT=COMPACT */
{
	return(dtype_get_fixed_size_low(col->mtype, col->prtype, col->len,
					col->mbminlen, col->mbmaxlen, comp));
}
/***********************************************************************//**
Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a column.
For fixed length types it is the fixed length of the type, otherwise 0.
@return SQL null storage size in ROW_FORMAT=REDUNDANT */
UNIV_INLINE
ulint
dict_col_get_sql_null_size(
/*=======================*/
	const dict_col_t*	col,	/*!< in: column */
	ulint			comp)	/*!< in: nonzero=ROW_FORMAT=COMPACT  */
{
	return(dict_col_get_fixed_size(col, comp));
}

/*********************************************************************//**
Gets the column number.
@return col->ind, table column position (starting from 0) */
UNIV_INLINE
ulint
dict_col_get_no(
/*============*/
	const dict_col_t*	col)	/*!< in: column */
{
	return(col->ind);
}

/*********************************************************************//**
Gets the column position in the clustered index. */
UNIV_INLINE
ulint
dict_col_get_clust_pos(
/*===================*/
	const dict_col_t*	col,		/*!< in: table column */
	const dict_index_t*	clust_index)	/*!< in: clustered index */
{
	ulint	i;

	ut_ad(dict_index_is_clust(clust_index));

	for (i = 0; i < clust_index->n_def; i++) {
		const dict_field_t*	field = &clust_index->fields[i];

		if (!field->prefix_len && field->col == col) {
			return(i);
		}
	}

	return(ULINT_UNDEFINED);
}

/** Gets the column position in the given index.
@param[in]	col	table column
@param[in]	index	index to be searched for column
@return position of column in the given index. */
UNIV_INLINE
ulint
dict_col_get_index_pos(
	const dict_col_t*	col,
	const dict_index_t*	index)
{
	ulint	i;

	for (i = 0; i < index->n_def; i++) {
		const dict_field_t*	field = &index->fields[i];

		if (!field->prefix_len && field->col == col) {
			return(i);
		}
	}

	return(ULINT_UNDEFINED);
}

#ifdef UNIV_DEBUG
/********************************************************************//**
Gets the first index on the table (the clustered index).
@return index, NULL if none exists */
UNIV_INLINE
dict_index_t*
dict_table_get_first_index(
/*=======================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

	return(UT_LIST_GET_FIRST(((dict_table_t*) table)->indexes));
}

/********************************************************************//**
Gets the last index on the table.
@return index, NULL if none exists */
UNIV_INLINE
dict_index_t*
dict_table_get_last_index(
/*=======================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	return(UT_LIST_GET_LAST((const_cast<dict_table_t*>(table))
				->indexes));
}

/********************************************************************//**
Gets the next index on the table.
@return index, NULL if none left */
UNIV_INLINE
dict_index_t*
dict_table_get_next_index(
/*======================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(UT_LIST_GET_NEXT(indexes, (dict_index_t*) index));
}
#endif /* UNIV_DEBUG */

/********************************************************************//**
Check whether the index is the clustered index.
@return nonzero for clustered index, zero for other indexes */
UNIV_INLINE
ulint
dict_index_is_clust(
/*================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(index->type & DICT_CLUSTERED);
}

/** Check if index is auto-generated clustered index.
@param[in]	index	index

@return true if index is auto-generated clustered index. */
UNIV_INLINE
bool
dict_index_is_auto_gen_clust(
	const dict_index_t*	index)
{
	return(index->type == DICT_CLUSTERED);
}

/********************************************************************//**
Check whether the index is unique.
@return nonzero for unique index, zero for other indexes */
UNIV_INLINE
ulint
dict_index_is_unique(
/*=================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(index->type & DICT_UNIQUE);
}

/********************************************************************//**
Check whether the index is a Spatial Index.
@return	nonzero for Spatial Index, zero for other indexes */
UNIV_INLINE
ulint
dict_index_is_spatial(
/*==================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(index->type & DICT_SPATIAL);
}

/********************************************************************//**
Check whether the index is the insert buffer tree.
@return nonzero for insert buffer, zero for other indexes */
UNIV_INLINE
ulint
dict_index_is_ibuf(
/*===============*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(index->type & DICT_IBUF);
}

/********************************************************************//**
Check whether the index is a secondary index or the insert buffer tree.
@return nonzero for insert buffer, zero for other indexes */
UNIV_INLINE
ulint
dict_index_is_sec_or_ibuf(
/*======================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return((index->type & (DICT_CLUSTERED | DICT_IBUF)) != DICT_CLUSTERED);
}

/********************************************************************//**
Gets the number of user-defined non-virtual columns in a table in the
dictionary cache.
@return number of user-defined (e.g., not ROW_ID) non-virtual
columns of a table */
UNIV_INLINE
ulint
dict_table_get_n_user_cols(
/*=======================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	return(table->n_cols - DATA_N_SYS_COLS);
}

/********************************************************************//**
Gets the number of all non-virtual columns (also system) in a table
in the dictionary cache.
@return number of non-virtual columns of a table */
UNIV_INLINE
ulint
dict_table_get_n_cols(
/*==================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	return(table->n_cols);
}

/** Gets the number of virtual columns in a table in the dictionary cache.
@param[in]	table	the table to check
@return number of virtual columns of a table */
UNIV_INLINE
ulint
dict_table_get_n_v_cols(
	const dict_table_t*	table)
{
	ut_ad(table);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

	return(table->n_v_cols);
}

/** Check if a table has indexed virtual columns
@param[in]	table	the table to check
@return true is the table has indexed virtual columns */
UNIV_INLINE
bool
dict_table_has_indexed_v_cols(
	const dict_table_t*	table)
{

	for (ulint i = 0; i < table->n_v_cols; i++) {
		const dict_v_col_t*     col = dict_table_get_nth_v_col(table, i);
		if (col->m_col.ord_part) {
			return(true);
		}
	}

	return(false);
}

/********************************************************************//**
Gets the approximately estimated number of rows in the table.
@return estimated number of rows */
UNIV_INLINE
ib_uint64_t
dict_table_get_n_rows(
/*==================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->stat_initialized);

	return(table->stat_n_rows);
}

/********************************************************************//**
Increment the number of rows in the table by one.
Notice that this operation is not protected by any latch, the number is
approximate. */
UNIV_INLINE
void
dict_table_n_rows_inc(
/*==================*/
	dict_table_t*	table)	/*!< in/out: table */
{
	if (table->stat_initialized) {
		ib_uint64_t	n_rows = table->stat_n_rows;
		if (n_rows < 0xFFFFFFFFFFFFFFFFULL) {
			table->stat_n_rows = n_rows + 1;
		}
	}
}

/********************************************************************//**
Decrement the number of rows in the table by one.
Notice that this operation is not protected by any latch, the number is
approximate. */
UNIV_INLINE
void
dict_table_n_rows_dec(
/*==================*/
	dict_table_t*	table)	/*!< in/out: table */
{
	if (table->stat_initialized) {
		ib_uint64_t	n_rows = table->stat_n_rows;
		if (n_rows > 0) {
			table->stat_n_rows = n_rows - 1;
		}
	}
}

#ifdef UNIV_DEBUG
/********************************************************************//**
Gets the nth column of a table.
@return pointer to column object */
UNIV_INLINE
dict_col_t*
dict_table_get_nth_col(
/*===================*/
	const dict_table_t*	table,	/*!< in: table */
	ulint			pos)	/*!< in: position of column */
{
	ut_ad(pos < table->n_def);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

	return((dict_col_t*) (table->cols) + pos);
}

/** Gets the nth virtual column of a table.
@param[in]	table	table
@param[in]	pos	position of virtual column
@return pointer to virtual column object */
UNIV_INLINE
dict_v_col_t*
dict_table_get_nth_v_col(
	const dict_table_t*	table,
	ulint			pos)
{
	ut_ad(table);
	ut_ad(pos < table->n_v_def);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

	return(static_cast<dict_v_col_t*>(table->v_cols) + pos);
}

/********************************************************************//**
Gets the given system column of a table.
@return pointer to column object */
UNIV_INLINE
dict_col_t*
dict_table_get_sys_col(
/*===================*/
	const dict_table_t*	table,	/*!< in: table */
	ulint			sys)	/*!< in: DATA_ROW_ID, ... */
{
	dict_col_t*	col;

	ut_ad(sys < DATA_N_SYS_COLS);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

	col = dict_table_get_nth_col(table, table->n_cols
				     + (sys - DATA_N_SYS_COLS));
	ut_ad(col->mtype == DATA_SYS);
	ut_ad(col->prtype == (sys | DATA_NOT_NULL));

	return(col);
}
#endif /* UNIV_DEBUG */

/********************************************************************//**
Gets the given system column number of a table.
@return column number */
UNIV_INLINE
ulint
dict_table_get_sys_col_no(
/*======================*/
	const dict_table_t*	table,	/*!< in: table */
	ulint			sys)	/*!< in: DATA_ROW_ID, ... */
{
	ut_ad(sys < DATA_N_SYS_COLS);
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	return table->n_cols + (sys - DATA_N_SYS_COLS);
}

/********************************************************************//**
Check whether the table uses the compact page format.
@return TRUE if table uses the compact page format */
UNIV_INLINE
ibool
dict_table_is_comp(
/*===============*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table);

#if DICT_TF_COMPACT != 1
#error "DICT_TF_COMPACT must be 1"
#endif

	return(table->flags & DICT_TF_COMPACT);
}

/************************************************************************
Check if the table has an FTS index. */
UNIV_INLINE
ibool
dict_table_has_fts_index(
/*=====================*/
				/* out: TRUE if table has an FTS index */
	dict_table_t*   table)  /* in: table */
{
	return(DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS));
}

/** Validate the flags for tables that are not ROW_FORMAT=REDUNDANT.
@param[in]	flags		table flags
@return whether the flags are valid */
inline
bool
dict_tf_is_valid_not_redundant(ulint flags)
{
	const bool	atomic_blobs = DICT_TF_HAS_ATOMIC_BLOBS(flags);

	ulint	zip_ssize = DICT_TF_GET_ZIP_SSIZE(flags);

	if (!zip_ssize) {
		/* Not ROW_FORMAT=COMPRESSED */
	} else if (!atomic_blobs) {
		/* ROW_FORMAT=COMPRESSED implies ROW_FORMAT=DYNAMIC
		for the uncompressed page format */
		return(false);
	} else if (zip_ssize > PAGE_ZIP_SSIZE_MAX
		   || zip_ssize > UNIV_PAGE_SIZE_SHIFT
		   || UNIV_PAGE_SIZE_SHIFT > UNIV_ZIP_SIZE_SHIFT_MAX) {
		/* KEY_BLOCK_SIZE is out of bounds, or
		ROW_FORMAT=COMPRESSED is not supported with this
		innodb_page_size (only up to 16KiB) */
		return(false);
	}

	switch (DICT_TF_GET_PAGE_COMPRESSION_LEVEL(flags)) {
	case 0:
		/* PAGE_COMPRESSION_LEVEL=0 should imply PAGE_COMPRESSED=NO */
		return(!DICT_TF_GET_PAGE_COMPRESSION(flags));
	case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
		/* PAGE_COMPRESSION_LEVEL requires
		ROW_FORMAT=COMPACT or ROW_FORMAT=DYNAMIC
		(not ROW_FORMAT=COMPRESSED or ROW_FORMAT=REDUNDANT)
		and PAGE_COMPRESSED=YES */
		return(!zip_ssize && DICT_TF_GET_PAGE_COMPRESSION(flags));
	default:
		/* Invalid PAGE_COMPRESSION_LEVEL value */
		return(false);
	}
}

/** Validate the table flags.
@param[in]	flags	Table flags
@return true if valid. */
UNIV_INLINE
bool
dict_tf_is_valid(
	ulint	flags)
{
	ut_ad(flags < 1U << DICT_TF_BITS);
	/* The DATA_DIRECTORY flag can be assigned fully independently
	of all other persistent table flags. */
	flags &= ~DICT_TF_MASK_DATA_DIR;
	if (!(flags & 1)) {
		/* Only ROW_FORMAT=REDUNDANT has 0 in the least significant
		bit. For ROW_FORMAT=REDUNDANT, only the DATA_DIR flag
		(which we cleared above) can be set. If any other flags
		are set, the flags are invalid. */
		return(flags == 0);
	}

	return(dict_tf_is_valid_not_redundant(flags));
}

/** Validate both table flags and table flags2 and make sure they
are compatible.
@param[in]	flags	Table flags
@param[in]	flags2	Table flags2
@return true if valid. */
UNIV_INLINE
bool
dict_tf2_is_valid(
	ulint	flags,
	ulint	flags2)
{
	if (!dict_tf_is_valid(flags)) {
		return(false);
	}

	if ((flags2 & DICT_TF2_UNUSED_BIT_MASK) != 0) {
		return(false);
	}

	return(true);
}

/********************************************************************//**
Determine the file format from dict_table_t::flags
The low order bit will be zero for REDUNDANT and 1 for COMPACT. For any
other row_format, file_format is > 0 and DICT_TF_COMPACT will also be set.
@return file format version */
UNIV_INLINE
rec_format_t
dict_tf_get_rec_format(
/*===================*/
	ulint		flags)	/*!< in: dict_table_t::flags */
{
	ut_a(dict_tf_is_valid(flags));

	if (!DICT_TF_GET_COMPACT(flags)) {
		return(REC_FORMAT_REDUNDANT);
	}

	if (!DICT_TF_HAS_ATOMIC_BLOBS(flags)) {
		return(REC_FORMAT_COMPACT);
	}

	if (DICT_TF_GET_ZIP_SSIZE(flags)) {
		return(REC_FORMAT_COMPRESSED);
	}

	return(REC_FORMAT_DYNAMIC);
}

/********************************************************************//**
Determine the file format from a dict_table_t::flags.
@return file format version */
UNIV_INLINE
ulint
dict_tf_get_format(
/*===============*/
	ulint		flags)	/*!< in: dict_table_t::flags */
{
	if (DICT_TF_HAS_ATOMIC_BLOBS(flags)) {
		return(UNIV_FORMAT_B);
	}

	return(UNIV_FORMAT_A);
}

/********************************************************************//**
Determine the file format of a table.
@return file format version */
UNIV_INLINE
ulint
dict_table_get_format(
/*==================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table);

	return(dict_tf_get_format(table->flags));
}

/** Set the various values in a dict_table_t::flags pointer.
@param[in,out]	flags,		Pointer to a 4 byte Table Flags
@param[in]	format		File Format
@param[in]	zip_ssize	Zip Shift Size
@param[in]	use_data_dir	Table uses DATA DIRECTORY
@param[in]	page_compressed Table uses page compression
@param[in]	page_compression_level Page compression level
@param[in]	not_used        For future */
UNIV_INLINE
void
dict_tf_set(
/*========*/
	ulint*		flags,
	rec_format_t	format,
	ulint		zip_ssize,
	bool		use_data_dir,
	bool		page_compressed,
	ulint		page_compression_level,
	ulint		not_used)
{
	*flags = use_data_dir ? 1 << DICT_TF_POS_DATA_DIR : 0;

	switch (format) {
	case REC_FORMAT_REDUNDANT:
		ut_ad(zip_ssize == 0);
		/* no other options are allowed */
		ut_ad(!page_compressed);
		return;
	case REC_FORMAT_COMPACT:
		*flags |= DICT_TF_COMPACT;
		ut_ad(zip_ssize == 0);
		break;
	case REC_FORMAT_COMPRESSED:
		*flags |= DICT_TF_COMPACT
			| (1 << DICT_TF_POS_ATOMIC_BLOBS)
			| (zip_ssize << DICT_TF_POS_ZIP_SSIZE);
		break;
	case REC_FORMAT_DYNAMIC:
		*flags |= DICT_TF_COMPACT
			| (1 << DICT_TF_POS_ATOMIC_BLOBS);
		ut_ad(zip_ssize == 0);
		break;
	}

	if (page_compressed) {
		*flags |= (1 << DICT_TF_POS_ATOMIC_BLOBS)
		       | (1 << DICT_TF_POS_PAGE_COMPRESSION)
		       | (page_compression_level << DICT_TF_POS_PAGE_COMPRESSION_LEVEL);

		ut_ad(zip_ssize == 0);
		ut_ad(dict_tf_get_page_compression(*flags) == TRUE);
		ut_ad(dict_tf_get_page_compression_level(*flags) == page_compression_level);
	}
}

/** Convert a 32 bit integer table flags to the 32 bit FSP Flags.
Fsp Flags are written into the tablespace header at the offset
FSP_SPACE_FLAGS and are also stored in the fil_space_t::flags field.
The following chart shows the translation of the low order bit.
Other bits are the same.
========================= Low order bit ==========================
                    | REDUNDANT | COMPACT | COMPRESSED | DYNAMIC
dict_table_t::flags |     0     |    1    |     1      |    1
fil_space_t::flags  |     0     |    0    |     1      |    1
==================================================================
@param[in]	table_flags	dict_table_t::flags
@return tablespace flags (fil_space_t::flags) */
UNIV_INLINE
ulint
dict_tf_to_fsp_flags(ulint table_flags)
{
	ulint fsp_flags;
	ulint page_compression_level = DICT_TF_GET_PAGE_COMPRESSION_LEVEL(
		table_flags);

	ut_ad((DICT_TF_GET_PAGE_COMPRESSION(table_flags) == 0)
	      == (page_compression_level == 0));

	DBUG_EXECUTE_IF("dict_tf_to_fsp_flags_failure",
			return(ULINT_UNDEFINED););

	/* Adjust bit zero. */
	fsp_flags = DICT_TF_HAS_ATOMIC_BLOBS(table_flags) ? 1 : 0;

	/* ZIP_SSIZE and ATOMIC_BLOBS are at the same position. */
	fsp_flags |= table_flags
		& (DICT_TF_MASK_ZIP_SSIZE | DICT_TF_MASK_ATOMIC_BLOBS);

	fsp_flags |= FSP_FLAGS_PAGE_SSIZE();

	if (page_compression_level) {
		fsp_flags |= FSP_FLAGS_MASK_PAGE_COMPRESSION;
	}

	ut_a(fsp_flags_is_valid(fsp_flags, false));

	if (DICT_TF_HAS_DATA_DIR(table_flags)) {
		fsp_flags |= 1U << FSP_FLAGS_MEM_DATA_DIR;
	}

	fsp_flags |= page_compression_level << FSP_FLAGS_MEM_COMPRESSION_LEVEL;

	return(fsp_flags);
}

/********************************************************************//**
Convert a 32 bit integer table flags to the 32bit integer that is written
to a SYS_TABLES.TYPE field. The following chart shows the translation of
the low order bit.  Other bits are the same.
========================= Low order bit ==========================
                    | REDUNDANT | COMPACT | COMPRESSED and DYNAMIC
dict_table_t::flags |     0     |    1    |     1
SYS_TABLES.TYPE     |     1     |    1    |     1
==================================================================
@return ulint containing SYS_TABLES.TYPE */
UNIV_INLINE
ulint
dict_tf_to_sys_tables_type(
/*=======================*/
	ulint	flags)	/*!< in: dict_table_t::flags */
{
	ulint type;

	ut_a(dict_tf_is_valid(flags));

	/* Adjust bit zero. It is always 1 in SYS_TABLES.TYPE */
	type = 1;

	/* ZIP_SSIZE, ATOMIC_BLOBS, DATA_DIR, PAGE_COMPRESSION,
	PAGE_COMPRESSION_LEVEL are the same. */
	type |= flags & (DICT_TF_MASK_ZIP_SSIZE
			 | DICT_TF_MASK_ATOMIC_BLOBS
			 | DICT_TF_MASK_DATA_DIR
			 | DICT_TF_MASK_PAGE_COMPRESSION
			 | DICT_TF_MASK_PAGE_COMPRESSION_LEVEL);

	return(type);
}

/** Extract the page size info from table flags.
@param[in]	flags	flags
@return a structure containing the compressed and uncompressed
page sizes and a boolean indicating if the page is compressed. */
UNIV_INLINE
const page_size_t
dict_tf_get_page_size(
	ulint	flags)
{
	const ulint	zip_ssize = DICT_TF_GET_ZIP_SSIZE(flags);

	if (zip_ssize == 0) {
		return(univ_page_size);
	}

	const ulint	zip_size = (UNIV_ZIP_SIZE_MIN >> 1) << zip_ssize;

	ut_ad(zip_size <= UNIV_ZIP_SIZE_MAX);

	return(page_size_t(zip_size, univ_page_size.logical(), true));
}

/** Get the table page size.
@param[in]	table	table
@return a structure containing the compressed and uncompressed
page sizes and a boolean indicating if the page is compressed */
UNIV_INLINE
const page_size_t
dict_table_page_size(
	const dict_table_t*	table)
{
	ut_ad(table != NULL);

	return(dict_tf_get_page_size(table->flags));
}

/*********************************************************************//**
Obtain exclusive locks on all index trees of the table. This is to prevent
accessing index trees while InnoDB is updating internal metadata for
operations such as truncate tables. */
UNIV_INLINE
void
dict_table_x_lock_indexes(
/*======================*/
	dict_table_t*	table)	/*!< in: table */
{
	ut_ad(mutex_own(&dict_sys->mutex));

	dict_index_t* clust_index = dict_table_get_first_index(table);

	/* Loop through each index of the table and lock them */
	for (dict_index_t* index = dict_table_get_next_index(clust_index);
	     index != NULL;
	     index = dict_table_get_next_index(index)) {
		rw_lock_x_lock(dict_index_get_lock(index));
	}

	rw_lock_x_lock(dict_index_get_lock(clust_index));
}

/*********************************************************************//**
Returns true if the particular FTS index in the table is still syncing
in the background, false otherwise.
@param [in] table      Table containing FTS index
@return True if sync of fts index is still going in the background  */
UNIV_INLINE
bool
dict_fts_index_syncing(
	dict_table_t*   table)
{
	 dict_index_t*   index;

	for (index = dict_table_get_first_index(table);
	    index != NULL;
	    index = dict_table_get_next_index(index)) {
		if (index->index_fts_syncing) {
			 return(true);
		}
	}
	return(false);
}
/*********************************************************************//**
Release the exclusive locks on all index tree. */
UNIV_INLINE
void
dict_table_x_unlock_indexes(
/*========================*/
	dict_table_t*	table)	/*!< in: table */
{
	dict_index_t*   index;

	ut_ad(mutex_own(&dict_sys->mutex));

	for (index = dict_table_get_first_index(table);
	     index != NULL;
	     index = dict_table_get_next_index(index)) {
		rw_lock_x_unlock(dict_index_get_lock(index));
	}
}

/********************************************************************//**
Gets the number of fields in the internal representation of an index,
including fields added by the dictionary system.
@return number of fields */
UNIV_INLINE
ulint
dict_index_get_n_fields(
/*====================*/
	const dict_index_t*	index)	/*!< in: an internal
					representation of index (in
					the dictionary cache) */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	return(index->n_fields);
}

/********************************************************************//**
Gets the number of fields in the internal representation of an index
that uniquely determine the position of an index entry in the index, if
we do not take multiversioning into account: in the B-tree use the value
returned by dict_index_get_n_unique_in_tree.
@return number of fields */
UNIV_INLINE
ulint
dict_index_get_n_unique(
/*====================*/
	const dict_index_t*	index)	/*!< in: an internal representation
					of index (in the dictionary cache) */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	ut_ad(index->cached);
	return(index->n_uniq);
}

/********************************************************************//**
Gets the number of fields in the internal representation of an index
which uniquely determine the position of an index entry in the index, if
we also take multiversioning into account.
@return number of fields */
UNIV_INLINE
ulint
dict_index_get_n_unique_in_tree(
/*============================*/
	const dict_index_t*	index)	/*!< in: an internal representation
					of index (in the dictionary cache) */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	ut_ad(index->cached);

	if (dict_index_is_clust(index)) {

		return(dict_index_get_n_unique(index));
	}

	return(dict_index_get_n_fields(index));
}

/**
Gets the number of fields on nonleaf page level in the internal representation
of an index which uniquely determine the position of an index entry in the
index, if we also take multiversioning into account. Note, it doesn't
include page no field.
@param[in]	index	index
@return number of fields */
UNIV_INLINE
ulint
dict_index_get_n_unique_in_tree_nonleaf(
	const dict_index_t*	index)
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	ut_ad(index->cached);

	if (dict_index_is_spatial(index)) {
		/* For spatial index, on non-leaf page, we have only
		2 fields(mbr+page_no). So, except page no field,
		there's one field there. */
		return(DICT_INDEX_SPATIAL_NODEPTR_SIZE);
	} else {
		return(dict_index_get_n_unique_in_tree(index));
	}
}

/********************************************************************//**
Gets the number of user-defined ordering fields in the index. In the internal
representation of clustered indexes we add the row id to the ordering fields
to make a clustered index unique, but this function returns the number of
fields the user defined in the index as ordering fields.
@return number of fields */
UNIV_INLINE
ulint
dict_index_get_n_ordering_defined_by_user(
/*======================================*/
	const dict_index_t*	index)	/*!< in: an internal representation
					of index (in the dictionary cache) */
{
	return(index->n_user_defined_cols);
}

#ifdef UNIV_DEBUG
/********************************************************************//**
Gets the nth field of an index.
@return pointer to field object */
UNIV_INLINE
dict_field_t*
dict_index_get_nth_field(
/*=====================*/
	const dict_index_t*	index,	/*!< in: index */
	ulint			pos)	/*!< in: position of field */
{
	ut_ad(pos < index->n_def);
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

	return((dict_field_t*) (index->fields) + pos);
}
#endif /* UNIV_DEBUG */

/********************************************************************//**
Returns the position of a system column in an index.
@return position, ULINT_UNDEFINED if not contained */
UNIV_INLINE
ulint
dict_index_get_sys_col_pos(
/*=======================*/
	const dict_index_t*	index,	/*!< in: index */
	ulint			type)	/*!< in: DATA_ROW_ID, ... */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
	ut_ad(!dict_index_is_ibuf(index));

	if (dict_index_is_clust(index)) {

		return(dict_col_get_clust_pos(
			       dict_table_get_sys_col(index->table, type),
			       index));
	}

	return(dict_index_get_nth_col_pos(
			index, dict_table_get_sys_col_no(index->table, type), NULL));
}

/*********************************************************************//**
Gets the field column.
@return field->col, pointer to the table column */
UNIV_INLINE
const dict_col_t*
dict_field_get_col(
/*===============*/
	const dict_field_t*	field)	/*!< in: index field */
{
	return(field->col);
}

/********************************************************************//**
Gets pointer to the nth column in an index.
@return column */
UNIV_INLINE
const dict_col_t*
dict_index_get_nth_col(
/*===================*/
	const dict_index_t*	index,	/*!< in: index */
	ulint			pos)	/*!< in: position of the field */
{
	return(dict_field_get_col(dict_index_get_nth_field(index, pos)));
}

/********************************************************************//**
Gets the column number the nth field in an index.
@return column number */
UNIV_INLINE
ulint
dict_index_get_nth_col_no(
/*======================*/
	const dict_index_t*	index,	/*!< in: index */
	ulint			pos)	/*!< in: position of the field */
{
	return(dict_col_get_no(dict_index_get_nth_col(index, pos)));
}

/********************************************************************//**
Looks for column n in an index.
@return position in internal representation of the index;
ULINT_UNDEFINED if not contained */
UNIV_INLINE
ulint
dict_index_get_nth_col_pos(
/*=======================*/
	const dict_index_t*	index,	/*!< in: index */
	ulint			n,	/*!< in: column number */
	ulint*			prefix_col_pos) /*!< out: col num if prefix */
{
	return(dict_index_get_nth_col_or_prefix_pos(index, n, false, false,
						    prefix_col_pos));
}

/********************************************************************//**
Returns the minimum data size of an index record.
@return minimum data size in bytes */
UNIV_INLINE
ulint
dict_index_get_min_size(
/*====================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ulint	n	= dict_index_get_n_fields(index);
	ulint	size	= 0;

	while (n--) {
		size += dict_col_get_min_size(dict_index_get_nth_col(index,
								     n));
	}

	return(size);
}

/*********************************************************************//**
Gets the space id of the root of the index tree.
@return space id */
UNIV_INLINE
ulint
dict_index_get_space(
/*=================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index);
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

	return(index->space);
}

/*********************************************************************//**
Sets the space id of the root of the index tree. */
UNIV_INLINE
void
dict_index_set_space(
/*=================*/
	dict_index_t*	index,	/*!< in/out: index */
	ulint		space)	/*!< in: space id */
{
	ut_ad(index);
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

	index->space = unsigned(space);
}

/*********************************************************************//**
Gets the page number of the root of the index tree.
@return page number */
UNIV_INLINE
ulint
dict_index_get_page(
/*================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

	return(index->page);
}

/*********************************************************************//**
Gets the read-write lock of the index tree.
@return read-write lock */
UNIV_INLINE
rw_lock_t*
dict_index_get_lock(
/*================*/
	const dict_index_t*	index)	/*!< in: index */
{
	ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

	return(&(index->lock));
}

/********************************************************************//**
Returns free space reserved for future updates of records. This is
relevant only in the case of many consecutive inserts, as updates
which make the records bigger might fragment the index.
@return number of free bytes on page, reserved for updates */
UNIV_INLINE
ulint
dict_index_get_space_reserve(void)
/*==============================*/
{
	return(UNIV_PAGE_SIZE / 16);
}

/********************************************************************//**
Gets the status of online index creation.
@return the status */
UNIV_INLINE
enum online_index_status
dict_index_get_online_status(
/*=========================*/
	const dict_index_t*	index)	/*!< in: secondary index */
{
	enum online_index_status	status;

	status = (enum online_index_status) index->online_status;

	/* Without the index->lock protection, the online
	status can change from ONLINE_INDEX_CREATION to
	ONLINE_INDEX_COMPLETE (or ONLINE_INDEX_ABORTED) in
	row_log_apply() once log application is done. So to make
	sure the status is ONLINE_INDEX_CREATION or ONLINE_INDEX_COMPLETE
	you should always do the recheck after acquiring index->lock */

#ifdef UNIV_DEBUG
	switch (status) {
	case ONLINE_INDEX_COMPLETE:
	case ONLINE_INDEX_CREATION:
	case ONLINE_INDEX_ABORTED:
	case ONLINE_INDEX_ABORTED_DROPPED:
		return(status);
	}
	ut_error;
#endif /* UNIV_DEBUG */
	return(status);
}

/********************************************************************//**
Sets the status of online index creation. */
UNIV_INLINE
void
dict_index_set_online_status(
/*=========================*/
	dict_index_t*			index,	/*!< in/out: index */
	enum online_index_status	status)	/*!< in: status */
{
	ut_ad(!(index->type & DICT_FTS));
	ut_ad(rw_lock_own(dict_index_get_lock(index), RW_LOCK_X));

#ifdef UNIV_DEBUG
	switch (dict_index_get_online_status(index)) {
	case ONLINE_INDEX_COMPLETE:
	case ONLINE_INDEX_CREATION:
		break;
	case ONLINE_INDEX_ABORTED:
		ut_ad(status == ONLINE_INDEX_ABORTED_DROPPED);
		break;
	case ONLINE_INDEX_ABORTED_DROPPED:
		ut_error;
	}
#endif /* UNIV_DEBUG */

	index->online_status = status;
	ut_ad(dict_index_get_online_status(index) == status);
}

/********************************************************************//**
Determines if a secondary index is being or has been created online,
or if the table is being rebuilt online, allowing concurrent modifications
to the table.
@retval true if the index is being or has been built online, or
if this is a clustered index and the table is being or has been rebuilt online
@retval false if the index has been created or the table has been
rebuilt completely */
UNIV_INLINE
bool
dict_index_is_online_ddl(
/*=====================*/
	const dict_index_t*	index)	/*!< in: index */
{
#ifdef UNIV_DEBUG
	if (dict_index_is_clust(index)) {
		switch (dict_index_get_online_status(index)) {
		case ONLINE_INDEX_CREATION:
			return(true);
		case ONLINE_INDEX_COMPLETE:
			return(false);
		case ONLINE_INDEX_ABORTED:
		case ONLINE_INDEX_ABORTED_DROPPED:
			break;
		}
		ut_ad(0);
		return(false);
	}
#endif /* UNIV_DEBUG */

	return(UNIV_UNLIKELY(dict_index_get_online_status(index)
			     != ONLINE_INDEX_COMPLETE));
}

/**********************************************************************//**
Check whether a column exists in an FTS index.
@return ULINT_UNDEFINED if no match else the offset within the vector */
UNIV_INLINE
ulint
dict_table_is_fts_column(
/*=====================*/
	ib_vector_t*	indexes,/*!< in: vector containing only FTS indexes */
	ulint		col_no,	/*!< in: col number to search for */
	bool		is_virtual) /*!< in: whether it is a virtual column */

{
	ulint		i;

	for (i = 0; i < ib_vector_size(indexes); ++i) {
		dict_index_t*	index;

		index = (dict_index_t*) ib_vector_getp(indexes, i);

		if (dict_index_contains_col_or_prefix(
			index, col_no, is_virtual)) {

			return(i);
		}
	}

	return(ULINT_UNDEFINED);
}

/**********************************************************************//**
Determine bytes of column prefix to be stored in the undo log. Please
note if the table format is UNIV_FORMAT_A (< UNIV_FORMAT_B), no prefix
needs to be stored in the undo log.
@return bytes of column prefix to be stored in the undo log */
UNIV_INLINE
ulint
dict_max_field_len_store_undo(
/*==========================*/
	dict_table_t*		table,	/*!< in: table */
	const dict_col_t*	col)	/*!< in: column which index prefix
					is based on */
{
	ulint	prefix_len = 0;

	if (dict_table_get_format(table) >= UNIV_FORMAT_B)
	{
		prefix_len = col->max_prefix
			? col->max_prefix
			: DICT_MAX_FIELD_LEN_BY_FORMAT(table);
	}

	return(prefix_len);
}

/** Determine maximum bytes of a virtual column need to be stored
in the undo log.
@param[in]	table		dict_table_t for the table
@param[in]	col_no		virtual column number
@return maximum bytes of virtual column to be stored in the undo log */
UNIV_INLINE
ulint
dict_max_v_field_len_store_undo(
	dict_table_t*		table,
	ulint			col_no)
{
	const dict_col_t*	col
		= &dict_table_get_nth_v_col(table, col_no)->m_col;
	ulint			max_log_len;

	/* This calculation conforms to the non-virtual column
	maximum log length calculation:
	1) for UNIV_FORMAT_A, upto REC_ANTELOPE_MAX_INDEX_COL_LEN
	for UNIV_FORMAT_B, upto col->max_prefix or
	2) REC_VERSION_56_MAX_INDEX_COL_LEN, whichever is less */
	if (dict_table_get_format(table) >= UNIV_FORMAT_B) {
		if (DATA_BIG_COL(col) && col->max_prefix > 0) {
			max_log_len = col->max_prefix;
		} else {
			max_log_len = DICT_MAX_FIELD_LEN_BY_FORMAT(table);
		}
	} else {
		max_log_len = REC_ANTELOPE_MAX_INDEX_COL_LEN;
	}

	return(max_log_len);
}

/**********************************************************************//**
Prevent table eviction by moving a table to the non-LRU list from the
LRU list if it is not already there. */
UNIV_INLINE
void
dict_table_prevent_eviction(
/*========================*/
	dict_table_t*	table)	/*!< in: table to prevent eviction */
{
	ut_ad(mutex_own(&dict_sys->mutex));
	if (table->can_be_evicted) {
		dict_table_move_from_lru_to_non_lru(table);
	}
}

/********************************************************************//**
Check whether the table is corrupted.
@return nonzero for corrupted table, zero for valid tables */
UNIV_INLINE
ulint
dict_table_is_corrupted(
/*====================*/
	const dict_table_t*	table)	/*!< in: table */
{
	ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
	return(table->corrupted);
}

/********************************************************************//**
Check if the tablespace for the table has been discarded.
@return true if the tablespace has been discarded. */
UNIV_INLINE
bool
dict_table_is_discarded(
/*====================*/
	const dict_table_t*	table)	/*!< in: table to check */
{
	return(DICT_TF2_FLAG_IS_SET(table, DICT_TF2_DISCARDED));
}

/** Check if the table is found is a file_per_table tablespace.
This test does not use table flags2 since some REDUNDANT tables in the
system tablespace may have garbage in the MIX_LEN field where flags2 is
stored. These garbage MIX_LEN fields were written before v3.23.52.
A patch was added to v3.23.52 which initializes the MIX_LEN field to 0.
Since file-per-table tablespaces were added in 4.1, any SYS_TABLES
record with a non-zero space ID will have a reliable MIX_LEN field.
However, this test does not use flags2 from SYS_TABLES.MIX_LEN.  Instead,
assume that if the tablespace is not a predefined system tablespace,
 then it must be file-per-table.
Also, during ALTER TABLE, the DICT_TF2_USE_FILE_PER_TABLE flag may not be
set on one of the file-per-table tablespaces.
This test cannot be done on a table in the process of being created
because the space_id will be zero until the tablespace is created.
@param[in]	table	An existing open table to check
@return true if this table was created as a file-per-table tablespace. */
UNIV_INLINE
bool
dict_table_is_file_per_table(
	const dict_table_t*	table)	/*!< in: table to check */
{
	return !is_system_tablespace(table->space);
}

/** Acquire the table handle. */
inline
void
dict_table_t::acquire()
{
	ut_ad(mutex_own(&dict_sys->mutex));
	my_atomic_add32_explicit(&n_ref_count, 1, MY_MEMORY_ORDER_RELAXED);
}

/** Release the table handle.
@return	whether the last handle was released */
inline
bool
dict_table_t::release()
{
	int32 n = my_atomic_add32_explicit(
			&n_ref_count, -1, MY_MEMORY_ORDER_RELAXED);
	ut_ad(n > 0);
	return n == 1;
}

/** Encode the number of columns and number of virtual columns in a
4 bytes value. We could do this because the number of columns in
InnoDB is limited to 1017
@param[in]      n_col   number of non-virtual column
@param[in]      n_v_col number of virtual column
@return encoded value */
UNIV_INLINE
ulint
dict_table_encode_n_col(
                ulint   n_col,
                ulint   n_v_col)
{
	return(n_col + (n_v_col<<16));
}

/** decode number of virtual and non-virtual columns in one 4 bytes value.
@param[in]      encoded encoded value
@param[in,out]     n_col   number of non-virtual column
@param[in,out]     n_v_col number of virtual column */
UNIV_INLINE
void
dict_table_decode_n_col(
                ulint   encoded,
                ulint*  n_col,
                ulint*  n_v_col)
{

	ulint	num = encoded & ~DICT_N_COLS_COMPACT;
	*n_v_col = num >> 16;
	*n_col = num & 0xFFFF;
}

/** Free the virtual column template
@param[in,out]	vc_templ	virtual column template */
void
dict_free_vc_templ(
	dict_vcol_templ_t*	vc_templ)
{
	UT_DELETE_ARRAY(vc_templ->default_rec);
	vc_templ->default_rec = NULL;

	if (vc_templ->vtempl != NULL) {
		ut_ad(vc_templ->n_v_col > 0);
		for (ulint i = 0; i < vc_templ->n_col
		     + vc_templ->n_v_col; i++) {
			if (vc_templ->vtempl[i] != NULL) {
				ut_free(vc_templ->vtempl[i]);
			}
		}
		ut_free(vc_templ->vtempl);
		vc_templ->vtempl = NULL;
	}
}

/** Check whether the table have virtual index.
@param[in]	table	InnoDB table
@return true if the table have virtual index, false otherwise. */
UNIV_INLINE
bool
dict_table_have_virtual_index(
	dict_table_t*	table)
{
	for (ulint col_no = 0; col_no < dict_table_get_n_v_cols(table);
	     col_no++) {
		const dict_v_col_t*	col
			= dict_table_get_nth_v_col(table, col_no);

		if (col->m_col.ord_part) {
			return(true);
		}
	}

	return(false);
}