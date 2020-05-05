#ifndef HYBRIDZIP_JPEG_CODEC_H
#define HYBRIDZIP_JPEG_CODEC_H

#include <cstdio>
#include <jpeglib.h>
#include <string>
#include <cstring>
#include <vector>
#include <jerror.h>
#include <jpegint.h>
#include "types.h"

class hzj_codec {
private:
    FILE *jpeg_file;

    void dct_arr_alloc(int **&arr) {
        arr = new int *[8];
        for (int i = 0; i < 8; i++) {
            arr[i] = new int[8];
        }
    }

    inline long hzj_round_up(long a, long b) {
        a += b - 1L;
        return a - (a % b);
    }

public:
    struct hzj_mcu_array {
        std::vector<int **> *coeff_arrays;
        uint8_t nchannels;

        hzj_mcu_array(uint8_t _nchannels) {
            nchannels = _nchannels;
            coeff_arrays = new std::vector<int **>[nchannels];
        }

        void push_array(uint8_t channel, int **arr) {
            coeff_arrays[channel].push_back(arr);
        }
    };

    struct hzj_header {
        struct COMP_SPEC {
            uint8_t component_id;
            uint8_t h_samp_factor;
            uint8_t v_samp_factor;
            uint8_t q_table_no;
        };

        struct QTABLE {
            uint16_t cof[0x40] = {0};
            bool is_empty = true;
        };

        uint16_t image_width;
        uint16_t image_height;
        uint8_t num_components;
        uint8_t jpeg_color_space;
        uint16_t output_width;
        uint16_t output_height;
        uint16_t min_DCT_h_scaled_size;
        uint16_t min_DCT_v_scaled_size;
        uint8_t data_precision;
        uint16_t CCIR601_sampling;

        QTABLE *qtables;
        std::vector<COMP_SPEC> comp_specs;

        // non-critical parameters
        uint8_t JFIF_major_version = 0;
        uint8_t JFIF_minor_version = 0;
        uint8_t density_unit;
        uint8_t X_density;
        uint8_t Y_density;


        hzj_header(j_decompress_ptr info) {
            image_width = info->image_width;
            image_height = info->image_height;
            num_components = info->num_components;
            jpeg_color_space = info->jpeg_color_space;
            output_width = info->output_width;
            output_height = info->output_height;
            min_DCT_v_scaled_size = info->min_DCT_v_scaled_size;
            min_DCT_h_scaled_size = info->min_DCT_h_scaled_size;
            data_precision = info->data_precision;
            CCIR601_sampling = info->CCIR601_sampling;

            qtables = new QTABLE[NUM_QUANT_TBLS];
            for (int tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) {
                if (info->quant_tbl_ptrs[tblno] != NULL) {
                    auto _qtable = info->quant_tbl_ptrs[tblno]->quantval;

                    auto qtable = QTABLE();
                    for (int i = 0; i < 0x40; i++) {
                        qtable.cof[i] = _qtable[i];
                    }

                    qtables[tblno] = qtable;
                    qtables[tblno].is_empty = false;
                }
            }

            jpeg_component_info *comp;
            int ci;
            for (ci = 0, comp = info->comp_info; ci < num_components; ci++, comp++) {
                COMP_SPEC spec{};
                spec.component_id = comp->component_id;
                spec.h_samp_factor = comp->h_samp_factor;
                spec.v_samp_factor = comp->v_samp_factor;
                spec.q_table_no = comp->quant_tbl_no;

                comp_specs.push_back(spec);
            }

        }

        void create_compress_ptr(j_compress_ptr info) {
            info->image_width = image_width;
            info->image_height = image_height;
            info->input_components = num_components;
            info->in_color_space = (J_COLOR_SPACE) jpeg_color_space;
#if JPEG_LIB_VERSION >= 70
            info->jpeg_width = output_width;
            info->jpeg_height = output_height;
            info->min_DCT_v_scaled_size = min_DCT_v_scaled_size;
            info->min_DCT_h_scaled_size = min_DCT_h_scaled_size;
#endif
            jpeg_set_defaults(info);
            jpeg_set_colorspace(info, (J_COLOR_SPACE) jpeg_color_space);

            info->data_precision = data_precision;
            info->CCIR601_sampling = CCIR601_sampling;

            JQUANT_TBL **qtblptr;
            for (int tblno = 0; tblno < NUM_QUANT_TBLS; tblno++) {
                if (!qtables[tblno].is_empty) {
                    qtblptr = &info->quant_tbl_ptrs[tblno];
                    if (*qtblptr == nullptr)
                        *qtblptr = jpeg_alloc_quant_table((j_common_ptr) info);
                    memcpy((*qtblptr)->quantval, qtables[tblno].cof,
                           sizeof((*qtblptr)->quantval));
                    (*qtblptr)->sent_table = FALSE;
                }
            }

            info->num_components = num_components;

            int ci;
            jpeg_component_info *comp;
            for (ci = 0, comp = info->comp_info; ci < num_components; ci++, comp++) {
                COMP_SPEC spec = comp_specs[ci];
                comp->component_id = spec.component_id;
                comp->h_samp_factor = spec.h_samp_factor;
                comp->v_samp_factor = spec.v_samp_factor;
                comp->quant_tbl_no = spec.q_table_no;
            }

            if (JFIF_major_version != 0 || JFIF_minor_version != 0) {
                if (JFIF_major_version == 1) {
                    info->JFIF_major_version = JFIF_major_version;
                    info->JFIF_minor_version = JFIF_minor_version;
                }
                info->density_unit = density_unit;
                info->X_density = X_density;
                info->Y_density = Y_density;
            }
        }
    };

    struct hzj_image_struct {
        hzj_header header;
        hzj_mcu_array mcus;
    };

    hzj_codec(uint8_t *buf, uint64_t len) {
        jpeg_file = fmemopen(buf, len, "rb");
    }

    hzj_codec(std::string filename) {
        jpeg_file = fopen(filename.c_str(), "rb");
    }

    hzj_image_struct read() {
        jpeg_decompress_struct info;
        jpeg_error_mgr err;


        info.err = jpeg_std_error(&err);
        jpeg_create_decompress(&info);

        jpeg_stdio_src(&info, jpeg_file);
        jpeg_read_header(&info, true);

        auto *coeff_arrays = jpeg_read_coefficients(&info);
        auto dct_coeffs = hzj_mcu_array(info.num_components);

        for (int ci = 0; ci < info.num_components; ci++) {
            jpeg_component_info *compptr = info.comp_info + ci;
            for (int by = 0; by < compptr->height_in_blocks; by++) {
                JBLOCKARRAY buffer = (info.mem->access_virt_barray)((j_common_ptr) &info, coeff_arrays[ci], by,
                                                                    (JDIMENSION) 1, FALSE);
                for (int bx = 0; bx < compptr->width_in_blocks; bx++) {
                    // We are now at a 8x8 dct-block.
                    int **dct_arr;
                    dct_arr_alloc(dct_arr);

                    for (int coeff_index = 0; coeff_index < 0x40; coeff_index++) {
                        dct_arr[coeff_index / 8][coeff_index % 8] = buffer[0][bx][coeff_index];
                    }
                    dct_coeffs.push_array(ci, dct_arr);
                }
            }
        }

        jpeg_finish_decompress(&info);
        jpeg_destroy_decompress(&info);


        return hzj_image_struct{hzj_header(&info), dct_coeffs};
    }

    void write(hzj_image_struct image, FILE *out) {
        j_compress_ptr info = new jpeg_compress_struct;
        jpeg_error_mgr err;

        jpeg_create_compress(info);


        image.header.create_compress_ptr(info);
        jpeg_stdio_dest(info, out);
        info->err = jpeg_std_error(&err);

        j_decompress_ptr dinfo = new jpeg_decompress_struct;
        jpeg_create_decompress(dinfo);


        uint8_t max_v_samp_factor = 1;
        uint8_t max_h_samp_factor = 1;
        for (int ci = 0; ci < image.header.num_components; ci++) {
            if (image.header.comp_specs[ci].v_samp_factor > max_v_samp_factor) {
                max_v_samp_factor = image.header.comp_specs[ci].v_samp_factor;
            }
            if (image.header.comp_specs[ci].h_samp_factor > max_h_samp_factor) {
                max_h_samp_factor = image.header.comp_specs[ci].h_samp_factor;
            }
        }

        auto *coeff_arrays = new jvirt_barray_ptr [info->num_components];

        jpeg_component_info *compptr;
        int ci;
        for (ci = 0, compptr = info->comp_info; ci < info->num_components; ci++, compptr++) {
            compptr->width_in_blocks = (JDIMENSION) image.header.image_height * image.header.comp_specs[ci].v_samp_factor /
                                       (8 * max_v_samp_factor);
            compptr->height_in_blocks = (JDIMENSION) image.header.image_width * image.header.comp_specs[ci].h_samp_factor /
                                        (8 * max_h_samp_factor);

            coeff_arrays[ci] = (*dinfo->mem->request_virt_barray)
                    ((j_common_ptr) dinfo, JPOOL_IMAGE, TRUE,
                     (JDIMENSION) hzj_round_up((long) compptr->width_in_blocks, (long) compptr->h_samp_factor),
                     (JDIMENSION) hzj_round_up((long) compptr->height_in_blocks, (long) compptr->v_samp_factor),
                     (JDIMENSION) compptr->v_samp_factor);
        }

        (*dinfo->mem->realize_virt_arrays)((j_common_ptr) dinfo);

        for (ci = 0, compptr = info->comp_info; ci < info->num_components; ci++, compptr++) {

            for (int by = 0; by < compptr->height_in_blocks; by++) {
                JBLOCKARRAY buffer = (*dinfo->mem->access_virt_barray)((j_common_ptr) dinfo, coeff_arrays[ci], by,
                                                                      (JDIMENSION) 1, TRUE);
                for (int bx = 0; bx < compptr->width_in_blocks; bx++) {
                    // We are now at a 8x8 dct-block.
                    int **dct_arr = image.mcus.coeff_arrays[ci][(by * compptr->width_in_blocks) + bx];

                    for (int coeff_index = 0; coeff_index < 0x40; coeff_index++) {
                        buffer[0][bx][coeff_index] = dct_arr[coeff_index / 8][coeff_index % 8];
                    }
                }
            }
        }

        jpeg_write_coefficients(info, coeff_arrays);
        jpeg_finish_compress(info);
        jpeg_destroy_compress(info);
    }


    void test() {

        std::string outname = "/home/supercmmetry/Pictures/supercmmetry.copy.jpg";
        jpeg_decompress_struct info;
        jpeg_error_mgr err;


        info.err = jpeg_std_error(&err);
        jpeg_create_decompress(&info);

        jpeg_stdio_src(&info, jpeg_file);
        jpeg_read_header(&info, true);


        auto *coeff_arrays = jpeg_read_coefficients(&info);


        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE * infile;

        if ((infile = fopen(outname.c_str(), "wb")) == NULL) {
            fprintf(stderr, "can't open %s\n", outname.c_str());
            return;
        }

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, infile);

        j_compress_ptr cinfo_ptr = &cinfo;
        jpeg_copy_critical_parameters((j_decompress_ptr)&info, cinfo_ptr);
        jpeg_write_coefficients(cinfo_ptr, coeff_arrays);
        jpeg_finish_compress(cinfo_ptr);
        jpeg_destroy_compress(cinfo_ptr);
    }
};

#endif