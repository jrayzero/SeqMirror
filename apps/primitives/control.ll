; ModuleID = 'control.c'
source_filename = "control.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.macroblock = type { i32, i32, i32, i32, i32*, i32*, i32*, i32*, %struct.video* }
%struct.video = type { i32, i32, i32, i32, i32, i32, i32*, i32*, i32* }

; Function Attrs: nounwind uwtable
define void @pred_luma_16x16(%struct.macroblock*) local_unnamed_addr #0 {
  %2 = tail call noalias i8* @malloc(i64 1024) #3
  %3 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 4
  %4 = bitcast i32** %3 to i8**
  store i8* %2, i8** %4, align 8, !tbaa !2
  %5 = tail call noalias i8* @malloc(i64 1024) #3
  %6 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 5
  %7 = bitcast i32** %6 to i8**
  store i8* %5, i8** %7, align 8, !tbaa !8
  %8 = tail call noalias i8* @malloc(i64 1024) #3
  %9 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 6
  %10 = bitcast i32** %9 to i8**
  store i8* %8, i8** %10, align 8, !tbaa !9
  %11 = tail call noalias i8* @malloc(i64 1024) #3
  %12 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 7
  %13 = bitcast i32** %12 to i8**
  store i8* %11, i8** %13, align 8, !tbaa !10
  %14 = tail call zeroext i1 @intra16x16_luma_vertical(%struct.macroblock* %0) #3
  %15 = tail call zeroext i1 @intra16x16_luma_horizontal(%struct.macroblock* %0) #3
  tail call void @intra16x16_luma_DC(%struct.macroblock* %0) #3
  %16 = tail call zeroext i1 @intra16x16_luma_planar(%struct.macroblock* %0) #3
  br i1 %14, label %17, label %26

; <label>:17:                                     ; preds = %1
  %18 = tail call noalias i8* @malloc(i64 1024) #3
  %19 = bitcast i8* %18 to i32*
  %20 = tail call noalias i8* @malloc(i64 1024) #3
  %21 = bitcast i8* %20 to i32*
  %22 = load i32*, i32** %3, align 8, !tbaa !2
  tail call void @xform_quant_luma_16x16(%struct.macroblock* nonnull %0, i32* %22, i32* %19, i32* %21)
  %23 = tail call i32 @sad(%struct.macroblock* nonnull %0, i32* %21) #3
  %24 = icmp eq i32 %23, 2147483647
  %25 = select i1 %24, i32* null, i32* %21
  br label %26

; <label>:26:                                     ; preds = %17, %1
  %27 = phi i32* [ %25, %17 ], [ null, %1 ]
  %28 = phi i32 [ %23, %17 ], [ 2147483647, %1 ]
  br i1 %15, label %29, label %39

; <label>:29:                                     ; preds = %26
  %30 = tail call noalias i8* @malloc(i64 1024) #3
  %31 = bitcast i8* %30 to i32*
  %32 = tail call noalias i8* @malloc(i64 1024) #3
  %33 = bitcast i8* %32 to i32*
  %34 = load i32*, i32** %6, align 8, !tbaa !8
  tail call void @xform_quant_luma_16x16(%struct.macroblock* nonnull %0, i32* %34, i32* %31, i32* %33)
  %35 = tail call i32 @sad(%struct.macroblock* nonnull %0, i32* %33) #3
  %36 = icmp slt i32 %35, %28
  %37 = select i1 %36, i32* %33, i32* %27
  %38 = select i1 %36, i32 %35, i32 %28
  br label %39

; <label>:39:                                     ; preds = %29, %26
  %40 = phi i32* [ %37, %29 ], [ %27, %26 ]
  %41 = phi i32 [ %38, %29 ], [ %28, %26 ]
  %42 = tail call noalias i8* @malloc(i64 1024) #3
  %43 = bitcast i8* %42 to i32*
  %44 = tail call noalias i8* @malloc(i64 1024) #3
  %45 = bitcast i8* %44 to i32*
  %46 = load i32*, i32** %9, align 8, !tbaa !9
  tail call void @xform_quant_luma_16x16(%struct.macroblock* nonnull %0, i32* %46, i32* %43, i32* %45)
  %47 = tail call i32 @sad(%struct.macroblock* nonnull %0, i32* %45) #3
  %48 = icmp slt i32 %47, %41
  %49 = select i1 %48, i32* %45, i32* %40
  br i1 %16, label %50, label %60

; <label>:50:                                     ; preds = %39
  %51 = select i1 %48, i32 %47, i32 %41
  %52 = tail call noalias i8* @malloc(i64 1024) #3
  %53 = bitcast i8* %52 to i32*
  %54 = tail call noalias i8* @malloc(i64 1024) #3
  %55 = bitcast i8* %54 to i32*
  %56 = load i32*, i32** %12, align 8, !tbaa !10
  tail call void @xform_quant_luma_16x16(%struct.macroblock* nonnull %0, i32* %56, i32* %53, i32* %55)
  %57 = tail call i32 @sad(%struct.macroblock* nonnull %0, i32* %55) #3
  %58 = icmp slt i32 %57, %51
  %59 = select i1 %58, i32* %55, i32* %49
  br label %60

; <label>:60:                                     ; preds = %50, %39
  %61 = phi i32* [ %59, %50 ], [ %49, %39 ]
  %62 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 8
  %63 = load %struct.video*, %struct.video** %62, align 8, !tbaa !11
  %64 = getelementptr inbounds %struct.video, %struct.video* %63, i64 0, i32 6
  %65 = load i32*, i32** %64, align 8, !tbaa !12
  %66 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 1
  %67 = getelementptr inbounds %struct.video, %struct.video* %63, i64 0, i32 1
  %68 = getelementptr inbounds %struct.video, %struct.video* %63, i64 0, i32 0
  %69 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 2
  %70 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 3
  br label %76

; <label>:71:                                     ; preds = %76
  %72 = load i8*, i8** %4, align 8, !tbaa !2
  tail call void @free(i8* %72) #3
  %73 = load i8*, i8** %7, align 8, !tbaa !8
  tail call void @free(i8* %73) #3
  %74 = load i8*, i8** %10, align 8, !tbaa !9
  tail call void @free(i8* %74) #3
  %75 = load i8*, i8** %13, align 8, !tbaa !10
  tail call void @free(i8* %75) #3
  ret void

; <label>:76:                                     ; preds = %76, %60
  %77 = phi i64 [ 0, %60 ], [ %334, %76 ]
  %78 = shl i64 %77, 4
  %79 = getelementptr inbounds i32, i32* %61, i64 %78
  %80 = load i32, i32* %79, align 4, !tbaa !14
  %81 = load i32, i32* %66, align 4, !tbaa !15
  %82 = load i32, i32* %67, align 4, !tbaa !16
  %83 = load i32, i32* %68, align 8, !tbaa !17
  %84 = mul i32 %83, %81
  %85 = load i32, i32* %69, align 8, !tbaa !18
  %86 = trunc i64 %77 to i32
  %87 = add nsw i32 %85, %86
  %88 = load i32, i32* %70, align 4, !tbaa !19
  %89 = add i32 %87, %84
  %90 = mul i32 %89, %82
  %91 = add i32 %88, %90
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds i32, i32* %65, i64 %92
  store i32 %80, i32* %93, align 4, !tbaa !14
  %94 = or i64 %78, 1
  %95 = getelementptr inbounds i32, i32* %61, i64 %94
  %96 = load i32, i32* %95, align 4, !tbaa !14
  %97 = load i32, i32* %66, align 4, !tbaa !15
  %98 = load i32, i32* %67, align 4, !tbaa !16
  %99 = load i32, i32* %68, align 8, !tbaa !17
  %100 = mul i32 %99, %97
  %101 = load i32, i32* %69, align 8, !tbaa !18
  %102 = add nsw i32 %101, %86
  %103 = load i32, i32* %70, align 4, !tbaa !19
  %104 = add i32 %102, %100
  %105 = mul i32 %104, %98
  %106 = add i32 %103, 1
  %107 = add i32 %106, %105
  %108 = sext i32 %107 to i64
  %109 = getelementptr inbounds i32, i32* %65, i64 %108
  store i32 %96, i32* %109, align 4, !tbaa !14
  %110 = or i64 %78, 2
  %111 = getelementptr inbounds i32, i32* %61, i64 %110
  %112 = load i32, i32* %111, align 4, !tbaa !14
  %113 = load i32, i32* %66, align 4, !tbaa !15
  %114 = load i32, i32* %67, align 4, !tbaa !16
  %115 = load i32, i32* %68, align 8, !tbaa !17
  %116 = mul i32 %115, %113
  %117 = load i32, i32* %69, align 8, !tbaa !18
  %118 = add nsw i32 %117, %86
  %119 = load i32, i32* %70, align 4, !tbaa !19
  %120 = add i32 %118, %116
  %121 = mul i32 %120, %114
  %122 = add i32 %119, 2
  %123 = add i32 %122, %121
  %124 = sext i32 %123 to i64
  %125 = getelementptr inbounds i32, i32* %65, i64 %124
  store i32 %112, i32* %125, align 4, !tbaa !14
  %126 = or i64 %78, 3
  %127 = getelementptr inbounds i32, i32* %61, i64 %126
  %128 = load i32, i32* %127, align 4, !tbaa !14
  %129 = load i32, i32* %66, align 4, !tbaa !15
  %130 = load i32, i32* %67, align 4, !tbaa !16
  %131 = load i32, i32* %68, align 8, !tbaa !17
  %132 = mul i32 %131, %129
  %133 = load i32, i32* %69, align 8, !tbaa !18
  %134 = add nsw i32 %133, %86
  %135 = load i32, i32* %70, align 4, !tbaa !19
  %136 = add i32 %134, %132
  %137 = mul i32 %136, %130
  %138 = add i32 %135, 3
  %139 = add i32 %138, %137
  %140 = sext i32 %139 to i64
  %141 = getelementptr inbounds i32, i32* %65, i64 %140
  store i32 %128, i32* %141, align 4, !tbaa !14
  %142 = or i64 %78, 4
  %143 = getelementptr inbounds i32, i32* %61, i64 %142
  %144 = load i32, i32* %143, align 4, !tbaa !14
  %145 = load i32, i32* %66, align 4, !tbaa !15
  %146 = load i32, i32* %67, align 4, !tbaa !16
  %147 = load i32, i32* %68, align 8, !tbaa !17
  %148 = mul i32 %147, %145
  %149 = load i32, i32* %69, align 8, !tbaa !18
  %150 = add nsw i32 %149, %86
  %151 = load i32, i32* %70, align 4, !tbaa !19
  %152 = add i32 %150, %148
  %153 = mul i32 %152, %146
  %154 = add i32 %151, 4
  %155 = add i32 %154, %153
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds i32, i32* %65, i64 %156
  store i32 %144, i32* %157, align 4, !tbaa !14
  %158 = or i64 %78, 5
  %159 = getelementptr inbounds i32, i32* %61, i64 %158
  %160 = load i32, i32* %159, align 4, !tbaa !14
  %161 = load i32, i32* %66, align 4, !tbaa !15
  %162 = load i32, i32* %67, align 4, !tbaa !16
  %163 = load i32, i32* %68, align 8, !tbaa !17
  %164 = mul i32 %163, %161
  %165 = load i32, i32* %69, align 8, !tbaa !18
  %166 = add nsw i32 %165, %86
  %167 = load i32, i32* %70, align 4, !tbaa !19
  %168 = add i32 %166, %164
  %169 = mul i32 %168, %162
  %170 = add i32 %167, 5
  %171 = add i32 %170, %169
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds i32, i32* %65, i64 %172
  store i32 %160, i32* %173, align 4, !tbaa !14
  %174 = or i64 %78, 6
  %175 = getelementptr inbounds i32, i32* %61, i64 %174
  %176 = load i32, i32* %175, align 4, !tbaa !14
  %177 = load i32, i32* %66, align 4, !tbaa !15
  %178 = load i32, i32* %67, align 4, !tbaa !16
  %179 = load i32, i32* %68, align 8, !tbaa !17
  %180 = mul i32 %179, %177
  %181 = load i32, i32* %69, align 8, !tbaa !18
  %182 = add nsw i32 %181, %86
  %183 = load i32, i32* %70, align 4, !tbaa !19
  %184 = add i32 %182, %180
  %185 = mul i32 %184, %178
  %186 = add i32 %183, 6
  %187 = add i32 %186, %185
  %188 = sext i32 %187 to i64
  %189 = getelementptr inbounds i32, i32* %65, i64 %188
  store i32 %176, i32* %189, align 4, !tbaa !14
  %190 = or i64 %78, 7
  %191 = getelementptr inbounds i32, i32* %61, i64 %190
  %192 = load i32, i32* %191, align 4, !tbaa !14
  %193 = load i32, i32* %66, align 4, !tbaa !15
  %194 = load i32, i32* %67, align 4, !tbaa !16
  %195 = load i32, i32* %68, align 8, !tbaa !17
  %196 = mul i32 %195, %193
  %197 = load i32, i32* %69, align 8, !tbaa !18
  %198 = add nsw i32 %197, %86
  %199 = load i32, i32* %70, align 4, !tbaa !19
  %200 = add i32 %198, %196
  %201 = mul i32 %200, %194
  %202 = add i32 %199, 7
  %203 = add i32 %202, %201
  %204 = sext i32 %203 to i64
  %205 = getelementptr inbounds i32, i32* %65, i64 %204
  store i32 %192, i32* %205, align 4, !tbaa !14
  %206 = or i64 %78, 8
  %207 = getelementptr inbounds i32, i32* %61, i64 %206
  %208 = load i32, i32* %207, align 4, !tbaa !14
  %209 = load i32, i32* %66, align 4, !tbaa !15
  %210 = load i32, i32* %67, align 4, !tbaa !16
  %211 = load i32, i32* %68, align 8, !tbaa !17
  %212 = mul i32 %211, %209
  %213 = load i32, i32* %69, align 8, !tbaa !18
  %214 = add nsw i32 %213, %86
  %215 = load i32, i32* %70, align 4, !tbaa !19
  %216 = add i32 %214, %212
  %217 = mul i32 %216, %210
  %218 = add i32 %215, 8
  %219 = add i32 %218, %217
  %220 = sext i32 %219 to i64
  %221 = getelementptr inbounds i32, i32* %65, i64 %220
  store i32 %208, i32* %221, align 4, !tbaa !14
  %222 = or i64 %78, 9
  %223 = getelementptr inbounds i32, i32* %61, i64 %222
  %224 = load i32, i32* %223, align 4, !tbaa !14
  %225 = load i32, i32* %66, align 4, !tbaa !15
  %226 = load i32, i32* %67, align 4, !tbaa !16
  %227 = load i32, i32* %68, align 8, !tbaa !17
  %228 = mul i32 %227, %225
  %229 = load i32, i32* %69, align 8, !tbaa !18
  %230 = add nsw i32 %229, %86
  %231 = load i32, i32* %70, align 4, !tbaa !19
  %232 = add i32 %230, %228
  %233 = mul i32 %232, %226
  %234 = add i32 %231, 9
  %235 = add i32 %234, %233
  %236 = sext i32 %235 to i64
  %237 = getelementptr inbounds i32, i32* %65, i64 %236
  store i32 %224, i32* %237, align 4, !tbaa !14
  %238 = or i64 %78, 10
  %239 = getelementptr inbounds i32, i32* %61, i64 %238
  %240 = load i32, i32* %239, align 4, !tbaa !14
  %241 = load i32, i32* %66, align 4, !tbaa !15
  %242 = load i32, i32* %67, align 4, !tbaa !16
  %243 = load i32, i32* %68, align 8, !tbaa !17
  %244 = mul i32 %243, %241
  %245 = load i32, i32* %69, align 8, !tbaa !18
  %246 = add nsw i32 %245, %86
  %247 = load i32, i32* %70, align 4, !tbaa !19
  %248 = add i32 %246, %244
  %249 = mul i32 %248, %242
  %250 = add i32 %247, 10
  %251 = add i32 %250, %249
  %252 = sext i32 %251 to i64
  %253 = getelementptr inbounds i32, i32* %65, i64 %252
  store i32 %240, i32* %253, align 4, !tbaa !14
  %254 = or i64 %78, 11
  %255 = getelementptr inbounds i32, i32* %61, i64 %254
  %256 = load i32, i32* %255, align 4, !tbaa !14
  %257 = load i32, i32* %66, align 4, !tbaa !15
  %258 = load i32, i32* %67, align 4, !tbaa !16
  %259 = load i32, i32* %68, align 8, !tbaa !17
  %260 = mul i32 %259, %257
  %261 = load i32, i32* %69, align 8, !tbaa !18
  %262 = add nsw i32 %261, %86
  %263 = load i32, i32* %70, align 4, !tbaa !19
  %264 = add i32 %262, %260
  %265 = mul i32 %264, %258
  %266 = add i32 %263, 11
  %267 = add i32 %266, %265
  %268 = sext i32 %267 to i64
  %269 = getelementptr inbounds i32, i32* %65, i64 %268
  store i32 %256, i32* %269, align 4, !tbaa !14
  %270 = or i64 %78, 12
  %271 = getelementptr inbounds i32, i32* %61, i64 %270
  %272 = load i32, i32* %271, align 4, !tbaa !14
  %273 = load i32, i32* %66, align 4, !tbaa !15
  %274 = load i32, i32* %67, align 4, !tbaa !16
  %275 = load i32, i32* %68, align 8, !tbaa !17
  %276 = mul i32 %275, %273
  %277 = load i32, i32* %69, align 8, !tbaa !18
  %278 = add nsw i32 %277, %86
  %279 = load i32, i32* %70, align 4, !tbaa !19
  %280 = add i32 %278, %276
  %281 = mul i32 %280, %274
  %282 = add i32 %279, 12
  %283 = add i32 %282, %281
  %284 = sext i32 %283 to i64
  %285 = getelementptr inbounds i32, i32* %65, i64 %284
  store i32 %272, i32* %285, align 4, !tbaa !14
  %286 = or i64 %78, 13
  %287 = getelementptr inbounds i32, i32* %61, i64 %286
  %288 = load i32, i32* %287, align 4, !tbaa !14
  %289 = load i32, i32* %66, align 4, !tbaa !15
  %290 = load i32, i32* %67, align 4, !tbaa !16
  %291 = load i32, i32* %68, align 8, !tbaa !17
  %292 = mul i32 %291, %289
  %293 = load i32, i32* %69, align 8, !tbaa !18
  %294 = add nsw i32 %293, %86
  %295 = load i32, i32* %70, align 4, !tbaa !19
  %296 = add i32 %294, %292
  %297 = mul i32 %296, %290
  %298 = add i32 %295, 13
  %299 = add i32 %298, %297
  %300 = sext i32 %299 to i64
  %301 = getelementptr inbounds i32, i32* %65, i64 %300
  store i32 %288, i32* %301, align 4, !tbaa !14
  %302 = or i64 %78, 14
  %303 = getelementptr inbounds i32, i32* %61, i64 %302
  %304 = load i32, i32* %303, align 4, !tbaa !14
  %305 = load i32, i32* %66, align 4, !tbaa !15
  %306 = load i32, i32* %67, align 4, !tbaa !16
  %307 = load i32, i32* %68, align 8, !tbaa !17
  %308 = mul i32 %307, %305
  %309 = load i32, i32* %69, align 8, !tbaa !18
  %310 = add nsw i32 %309, %86
  %311 = load i32, i32* %70, align 4, !tbaa !19
  %312 = add i32 %310, %308
  %313 = mul i32 %312, %306
  %314 = add i32 %311, 14
  %315 = add i32 %314, %313
  %316 = sext i32 %315 to i64
  %317 = getelementptr inbounds i32, i32* %65, i64 %316
  store i32 %304, i32* %317, align 4, !tbaa !14
  %318 = or i64 %78, 15
  %319 = getelementptr inbounds i32, i32* %61, i64 %318
  %320 = load i32, i32* %319, align 4, !tbaa !14
  %321 = load i32, i32* %66, align 4, !tbaa !15
  %322 = load i32, i32* %67, align 4, !tbaa !16
  %323 = load i32, i32* %68, align 8, !tbaa !17
  %324 = mul i32 %323, %321
  %325 = load i32, i32* %69, align 8, !tbaa !18
  %326 = add nsw i32 %325, %86
  %327 = load i32, i32* %70, align 4, !tbaa !19
  %328 = add i32 %326, %324
  %329 = mul i32 %328, %322
  %330 = add i32 %327, 15
  %331 = add i32 %330, %329
  %332 = sext i32 %331 to i64
  %333 = getelementptr inbounds i32, i32* %65, i64 %332
  store i32 %320, i32* %333, align 4, !tbaa !14
  %334 = add nuw nsw i64 %77, 1
  %335 = icmp eq i64 %334, 16
  br i1 %335, label %71, label %76
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) local_unnamed_addr #1

declare zeroext i1 @intra16x16_luma_vertical(%struct.macroblock*) local_unnamed_addr #2

declare zeroext i1 @intra16x16_luma_horizontal(%struct.macroblock*) local_unnamed_addr #2

declare void @intra16x16_luma_DC(%struct.macroblock*) local_unnamed_addr #2

declare zeroext i1 @intra16x16_luma_planar(%struct.macroblock*) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define void @xform_quant_luma_16x16(%struct.macroblock* nocapture readonly, i32* nocapture readonly, i32*, i32*) local_unnamed_addr #0 {
  %5 = tail call noalias i8* @malloc(i64 1024) #3
  %6 = bitcast i8* %5 to i32*
  %7 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 8
  %8 = load %struct.video*, %struct.video** %7, align 8, !tbaa !11
  %9 = getelementptr inbounds %struct.video, %struct.video* %8, i64 0, i32 6
  %10 = load i32*, i32** %9, align 8, !tbaa !12
  %11 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 1
  %12 = getelementptr inbounds %struct.video, %struct.video* %8, i64 0, i32 1
  %13 = getelementptr inbounds %struct.video, %struct.video* %8, i64 0, i32 0
  %14 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 2
  %15 = getelementptr inbounds %struct.macroblock, %struct.macroblock* %0, i64 0, i32 3
  %16 = load i32, i32* %11, align 4, !tbaa !15
  %17 = load i32, i32* %14, align 8, !tbaa !18
  %18 = load i32, i32* %15, align 4, !tbaa !19
  %19 = add i32 %18, 1
  %20 = add i32 %18, 2
  %21 = add i32 %18, 3
  %22 = add i32 %18, 4
  %23 = add i32 %18, 5
  %24 = add i32 %18, 6
  %25 = add i32 %18, 7
  %26 = add i32 %18, 8
  %27 = add i32 %18, 9
  %28 = add i32 %18, 10
  %29 = add i32 %18, 11
  %30 = add i32 %18, 12
  %31 = add i32 %18, 13
  %32 = add i32 %18, 14
  %33 = add i32 %18, 15
  br label %38

; <label>:34:                                     ; preds = %38
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 0, i32 0) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 0, i32 4) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 0, i32 8) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 0, i32 12) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 4, i32 0) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 4, i32 4) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 4, i32 8) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 4, i32 12) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 8, i32 0) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 8, i32 4) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 8, i32 8) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 8, i32 12) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 12, i32 0) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 12, i32 4) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 12, i32 8) #3
  tail call void @forward4x4(i32* nonnull %6, i32* %2, i32 12, i32 12) #3
  %35 = tail call noalias i8* @malloc(i64 64) #3
  %36 = bitcast i8* %35 to i32*
  tail call void @hadamard4x4_DCs(i32* %2, i32* %36) #3
  %37 = tail call zeroext i1 @quant_dc4x4_normal(i32* %36, i32 28) #3
  br i1 %37, label %268, label %285

; <label>:38:                                     ; preds = %38, %4
  %39 = phi i64 [ 0, %4 ], [ %266, %38 ]
  %40 = shl i64 %39, 4
  %41 = trunc i64 %39 to i32
  %42 = add nsw i32 %17, %41
  %43 = load i32, i32* %12, align 4, !tbaa !16
  %44 = load i32, i32* %13, align 8, !tbaa !17
  %45 = mul i32 %44, %16
  %46 = add i32 %42, %45
  %47 = mul i32 %46, %43
  %48 = add i32 %18, %47
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds i32, i32* %10, i64 %49
  %51 = load i32, i32* %50, align 4, !tbaa !14
  %52 = getelementptr inbounds i32, i32* %1, i64 %40
  %53 = load i32, i32* %52, align 4, !tbaa !14
  %54 = sub nsw i32 %51, %53
  %55 = getelementptr inbounds i32, i32* %6, i64 %40
  store i32 %54, i32* %55, align 4, !tbaa !14
  %56 = or i64 %40, 1
  %57 = load i32, i32* %12, align 4, !tbaa !16
  %58 = load i32, i32* %13, align 8, !tbaa !17
  %59 = mul i32 %58, %16
  %60 = add i32 %42, %59
  %61 = mul i32 %60, %57
  %62 = add i32 %19, %61
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds i32, i32* %10, i64 %63
  %65 = load i32, i32* %64, align 4, !tbaa !14
  %66 = getelementptr inbounds i32, i32* %1, i64 %56
  %67 = load i32, i32* %66, align 4, !tbaa !14
  %68 = sub nsw i32 %65, %67
  %69 = getelementptr inbounds i32, i32* %6, i64 %56
  store i32 %68, i32* %69, align 4, !tbaa !14
  %70 = or i64 %40, 2
  %71 = load i32, i32* %12, align 4, !tbaa !16
  %72 = load i32, i32* %13, align 8, !tbaa !17
  %73 = mul i32 %72, %16
  %74 = add i32 %42, %73
  %75 = mul i32 %74, %71
  %76 = add i32 %20, %75
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds i32, i32* %10, i64 %77
  %79 = load i32, i32* %78, align 4, !tbaa !14
  %80 = getelementptr inbounds i32, i32* %1, i64 %70
  %81 = load i32, i32* %80, align 4, !tbaa !14
  %82 = sub nsw i32 %79, %81
  %83 = getelementptr inbounds i32, i32* %6, i64 %70
  store i32 %82, i32* %83, align 4, !tbaa !14
  %84 = or i64 %40, 3
  %85 = load i32, i32* %12, align 4, !tbaa !16
  %86 = load i32, i32* %13, align 8, !tbaa !17
  %87 = mul i32 %86, %16
  %88 = add i32 %42, %87
  %89 = mul i32 %88, %85
  %90 = add i32 %21, %89
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds i32, i32* %10, i64 %91
  %93 = load i32, i32* %92, align 4, !tbaa !14
  %94 = getelementptr inbounds i32, i32* %1, i64 %84
  %95 = load i32, i32* %94, align 4, !tbaa !14
  %96 = sub nsw i32 %93, %95
  %97 = getelementptr inbounds i32, i32* %6, i64 %84
  store i32 %96, i32* %97, align 4, !tbaa !14
  %98 = or i64 %40, 4
  %99 = load i32, i32* %12, align 4, !tbaa !16
  %100 = load i32, i32* %13, align 8, !tbaa !17
  %101 = mul i32 %100, %16
  %102 = add i32 %42, %101
  %103 = mul i32 %102, %99
  %104 = add i32 %22, %103
  %105 = sext i32 %104 to i64
  %106 = getelementptr inbounds i32, i32* %10, i64 %105
  %107 = load i32, i32* %106, align 4, !tbaa !14
  %108 = getelementptr inbounds i32, i32* %1, i64 %98
  %109 = load i32, i32* %108, align 4, !tbaa !14
  %110 = sub nsw i32 %107, %109
  %111 = getelementptr inbounds i32, i32* %6, i64 %98
  store i32 %110, i32* %111, align 4, !tbaa !14
  %112 = or i64 %40, 5
  %113 = load i32, i32* %12, align 4, !tbaa !16
  %114 = load i32, i32* %13, align 8, !tbaa !17
  %115 = mul i32 %114, %16
  %116 = add i32 %42, %115
  %117 = mul i32 %116, %113
  %118 = add i32 %23, %117
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds i32, i32* %10, i64 %119
  %121 = load i32, i32* %120, align 4, !tbaa !14
  %122 = getelementptr inbounds i32, i32* %1, i64 %112
  %123 = load i32, i32* %122, align 4, !tbaa !14
  %124 = sub nsw i32 %121, %123
  %125 = getelementptr inbounds i32, i32* %6, i64 %112
  store i32 %124, i32* %125, align 4, !tbaa !14
  %126 = or i64 %40, 6
  %127 = load i32, i32* %12, align 4, !tbaa !16
  %128 = load i32, i32* %13, align 8, !tbaa !17
  %129 = mul i32 %128, %16
  %130 = add i32 %42, %129
  %131 = mul i32 %130, %127
  %132 = add i32 %24, %131
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds i32, i32* %10, i64 %133
  %135 = load i32, i32* %134, align 4, !tbaa !14
  %136 = getelementptr inbounds i32, i32* %1, i64 %126
  %137 = load i32, i32* %136, align 4, !tbaa !14
  %138 = sub nsw i32 %135, %137
  %139 = getelementptr inbounds i32, i32* %6, i64 %126
  store i32 %138, i32* %139, align 4, !tbaa !14
  %140 = or i64 %40, 7
  %141 = load i32, i32* %12, align 4, !tbaa !16
  %142 = load i32, i32* %13, align 8, !tbaa !17
  %143 = mul i32 %142, %16
  %144 = add i32 %42, %143
  %145 = mul i32 %144, %141
  %146 = add i32 %25, %145
  %147 = sext i32 %146 to i64
  %148 = getelementptr inbounds i32, i32* %10, i64 %147
  %149 = load i32, i32* %148, align 4, !tbaa !14
  %150 = getelementptr inbounds i32, i32* %1, i64 %140
  %151 = load i32, i32* %150, align 4, !tbaa !14
  %152 = sub nsw i32 %149, %151
  %153 = getelementptr inbounds i32, i32* %6, i64 %140
  store i32 %152, i32* %153, align 4, !tbaa !14
  %154 = or i64 %40, 8
  %155 = load i32, i32* %12, align 4, !tbaa !16
  %156 = load i32, i32* %13, align 8, !tbaa !17
  %157 = mul i32 %156, %16
  %158 = add i32 %42, %157
  %159 = mul i32 %158, %155
  %160 = add i32 %26, %159
  %161 = sext i32 %160 to i64
  %162 = getelementptr inbounds i32, i32* %10, i64 %161
  %163 = load i32, i32* %162, align 4, !tbaa !14
  %164 = getelementptr inbounds i32, i32* %1, i64 %154
  %165 = load i32, i32* %164, align 4, !tbaa !14
  %166 = sub nsw i32 %163, %165
  %167 = getelementptr inbounds i32, i32* %6, i64 %154
  store i32 %166, i32* %167, align 4, !tbaa !14
  %168 = or i64 %40, 9
  %169 = load i32, i32* %12, align 4, !tbaa !16
  %170 = load i32, i32* %13, align 8, !tbaa !17
  %171 = mul i32 %170, %16
  %172 = add i32 %42, %171
  %173 = mul i32 %172, %169
  %174 = add i32 %27, %173
  %175 = sext i32 %174 to i64
  %176 = getelementptr inbounds i32, i32* %10, i64 %175
  %177 = load i32, i32* %176, align 4, !tbaa !14
  %178 = getelementptr inbounds i32, i32* %1, i64 %168
  %179 = load i32, i32* %178, align 4, !tbaa !14
  %180 = sub nsw i32 %177, %179
  %181 = getelementptr inbounds i32, i32* %6, i64 %168
  store i32 %180, i32* %181, align 4, !tbaa !14
  %182 = or i64 %40, 10
  %183 = load i32, i32* %12, align 4, !tbaa !16
  %184 = load i32, i32* %13, align 8, !tbaa !17
  %185 = mul i32 %184, %16
  %186 = add i32 %42, %185
  %187 = mul i32 %186, %183
  %188 = add i32 %28, %187
  %189 = sext i32 %188 to i64
  %190 = getelementptr inbounds i32, i32* %10, i64 %189
  %191 = load i32, i32* %190, align 4, !tbaa !14
  %192 = getelementptr inbounds i32, i32* %1, i64 %182
  %193 = load i32, i32* %192, align 4, !tbaa !14
  %194 = sub nsw i32 %191, %193
  %195 = getelementptr inbounds i32, i32* %6, i64 %182
  store i32 %194, i32* %195, align 4, !tbaa !14
  %196 = or i64 %40, 11
  %197 = load i32, i32* %12, align 4, !tbaa !16
  %198 = load i32, i32* %13, align 8, !tbaa !17
  %199 = mul i32 %198, %16
  %200 = add i32 %42, %199
  %201 = mul i32 %200, %197
  %202 = add i32 %29, %201
  %203 = sext i32 %202 to i64
  %204 = getelementptr inbounds i32, i32* %10, i64 %203
  %205 = load i32, i32* %204, align 4, !tbaa !14
  %206 = getelementptr inbounds i32, i32* %1, i64 %196
  %207 = load i32, i32* %206, align 4, !tbaa !14
  %208 = sub nsw i32 %205, %207
  %209 = getelementptr inbounds i32, i32* %6, i64 %196
  store i32 %208, i32* %209, align 4, !tbaa !14
  %210 = or i64 %40, 12
  %211 = load i32, i32* %12, align 4, !tbaa !16
  %212 = load i32, i32* %13, align 8, !tbaa !17
  %213 = mul i32 %212, %16
  %214 = add i32 %42, %213
  %215 = mul i32 %214, %211
  %216 = add i32 %30, %215
  %217 = sext i32 %216 to i64
  %218 = getelementptr inbounds i32, i32* %10, i64 %217
  %219 = load i32, i32* %218, align 4, !tbaa !14
  %220 = getelementptr inbounds i32, i32* %1, i64 %210
  %221 = load i32, i32* %220, align 4, !tbaa !14
  %222 = sub nsw i32 %219, %221
  %223 = getelementptr inbounds i32, i32* %6, i64 %210
  store i32 %222, i32* %223, align 4, !tbaa !14
  %224 = or i64 %40, 13
  %225 = load i32, i32* %12, align 4, !tbaa !16
  %226 = load i32, i32* %13, align 8, !tbaa !17
  %227 = mul i32 %226, %16
  %228 = add i32 %42, %227
  %229 = mul i32 %228, %225
  %230 = add i32 %31, %229
  %231 = sext i32 %230 to i64
  %232 = getelementptr inbounds i32, i32* %10, i64 %231
  %233 = load i32, i32* %232, align 4, !tbaa !14
  %234 = getelementptr inbounds i32, i32* %1, i64 %224
  %235 = load i32, i32* %234, align 4, !tbaa !14
  %236 = sub nsw i32 %233, %235
  %237 = getelementptr inbounds i32, i32* %6, i64 %224
  store i32 %236, i32* %237, align 4, !tbaa !14
  %238 = or i64 %40, 14
  %239 = load i32, i32* %12, align 4, !tbaa !16
  %240 = load i32, i32* %13, align 8, !tbaa !17
  %241 = mul i32 %240, %16
  %242 = add i32 %42, %241
  %243 = mul i32 %242, %239
  %244 = add i32 %32, %243
  %245 = sext i32 %244 to i64
  %246 = getelementptr inbounds i32, i32* %10, i64 %245
  %247 = load i32, i32* %246, align 4, !tbaa !14
  %248 = getelementptr inbounds i32, i32* %1, i64 %238
  %249 = load i32, i32* %248, align 4, !tbaa !14
  %250 = sub nsw i32 %247, %249
  %251 = getelementptr inbounds i32, i32* %6, i64 %238
  store i32 %250, i32* %251, align 4, !tbaa !14
  %252 = or i64 %40, 15
  %253 = load i32, i32* %12, align 4, !tbaa !16
  %254 = load i32, i32* %13, align 8, !tbaa !17
  %255 = mul i32 %254, %16
  %256 = add i32 %42, %255
  %257 = mul i32 %256, %253
  %258 = add i32 %33, %257
  %259 = sext i32 %258 to i64
  %260 = getelementptr inbounds i32, i32* %10, i64 %259
  %261 = load i32, i32* %260, align 4, !tbaa !14
  %262 = getelementptr inbounds i32, i32* %1, i64 %252
  %263 = load i32, i32* %262, align 4, !tbaa !14
  %264 = sub nsw i32 %261, %263
  %265 = getelementptr inbounds i32, i32* %6, i64 %252
  store i32 %264, i32* %265, align 4, !tbaa !14
  %266 = add nuw nsw i64 %39, 1
  %267 = icmp eq i64 %266, 16
  br i1 %267, label %34, label %38

; <label>:268:                                    ; preds = %34
  tail call void @ihadamard4x4_DCs(i32* %36, i32* %3) #3
  tail call void @inverse_quant_DCs(i32* %3, i32 28) #3
  %269 = load i32, i32* %3, align 4, !tbaa !14
  %270 = getelementptr inbounds i32, i32* %3, i64 4
  %271 = getelementptr inbounds i32, i32* %3, i64 8
  %272 = getelementptr inbounds i32, i32* %3, i64 12
  %273 = getelementptr inbounds i32, i32* %3, i64 64
  %274 = getelementptr inbounds i32, i32* %3, i64 68
  %275 = getelementptr inbounds i32, i32* %3, i64 72
  %276 = getelementptr inbounds i32, i32* %3, i64 76
  %277 = getelementptr inbounds i32, i32* %3, i64 128
  %278 = getelementptr inbounds i32, i32* %3, i64 132
  %279 = getelementptr inbounds i32, i32* %3, i64 136
  %280 = getelementptr inbounds i32, i32* %3, i64 140
  %281 = getelementptr inbounds i32, i32* %3, i64 192
  %282 = getelementptr inbounds i32, i32* %3, i64 196
  %283 = getelementptr inbounds i32, i32* %3, i64 200
  %284 = getelementptr inbounds i32, i32* %3, i64 204
  br label %301

; <label>:285:                                    ; preds = %34
  store i32 0, i32* %3, align 4, !tbaa !14
  %286 = getelementptr inbounds i32, i32* %3, i64 4
  store i32 0, i32* %286, align 4, !tbaa !14
  %287 = getelementptr inbounds i32, i32* %3, i64 8
  store i32 0, i32* %287, align 4, !tbaa !14
  %288 = getelementptr inbounds i32, i32* %3, i64 12
  store i32 0, i32* %288, align 4, !tbaa !14
  %289 = getelementptr inbounds i32, i32* %3, i64 64
  store i32 0, i32* %289, align 4, !tbaa !14
  %290 = getelementptr inbounds i32, i32* %3, i64 68
  store i32 0, i32* %290, align 4, !tbaa !14
  %291 = getelementptr inbounds i32, i32* %3, i64 72
  store i32 0, i32* %291, align 4, !tbaa !14
  %292 = getelementptr inbounds i32, i32* %3, i64 76
  store i32 0, i32* %292, align 4, !tbaa !14
  %293 = getelementptr inbounds i32, i32* %3, i64 128
  store i32 0, i32* %293, align 4, !tbaa !14
  %294 = getelementptr inbounds i32, i32* %3, i64 132
  store i32 0, i32* %294, align 4, !tbaa !14
  %295 = getelementptr inbounds i32, i32* %3, i64 136
  store i32 0, i32* %295, align 4, !tbaa !14
  %296 = getelementptr inbounds i32, i32* %3, i64 140
  store i32 0, i32* %296, align 4, !tbaa !14
  %297 = getelementptr inbounds i32, i32* %3, i64 192
  store i32 0, i32* %297, align 4, !tbaa !14
  %298 = getelementptr inbounds i32, i32* %3, i64 196
  store i32 0, i32* %298, align 4, !tbaa !14
  %299 = getelementptr inbounds i32, i32* %3, i64 200
  store i32 0, i32* %299, align 4, !tbaa !14
  %300 = getelementptr inbounds i32, i32* %3, i64 204
  store i32 0, i32* %300, align 4, !tbaa !14
  br label %301

; <label>:301:                                    ; preds = %285, %268
  %302 = phi i32* [ %300, %285 ], [ %284, %268 ]
  %303 = phi i32* [ %299, %285 ], [ %283, %268 ]
  %304 = phi i32* [ %298, %285 ], [ %282, %268 ]
  %305 = phi i32* [ %297, %285 ], [ %281, %268 ]
  %306 = phi i32* [ %296, %285 ], [ %280, %268 ]
  %307 = phi i32* [ %295, %285 ], [ %279, %268 ]
  %308 = phi i32* [ %294, %285 ], [ %278, %268 ]
  %309 = phi i32* [ %293, %285 ], [ %277, %268 ]
  %310 = phi i32* [ %292, %285 ], [ %276, %268 ]
  %311 = phi i32* [ %291, %285 ], [ %275, %268 ]
  %312 = phi i32* [ %290, %285 ], [ %274, %268 ]
  %313 = phi i32* [ %289, %285 ], [ %273, %268 ]
  %314 = phi i32* [ %288, %285 ], [ %272, %268 ]
  %315 = phi i32* [ %287, %285 ], [ %271, %268 ]
  %316 = phi i32* [ %286, %285 ], [ %270, %268 ]
  %317 = phi i32 [ 0, %285 ], [ %269, %268 ]
  store i32 %317, i32* %2, align 4, !tbaa !14
  %318 = load i32, i32* %316, align 4, !tbaa !14
  %319 = getelementptr inbounds i32, i32* %2, i64 4
  store i32 %318, i32* %319, align 4, !tbaa !14
  %320 = load i32, i32* %315, align 4, !tbaa !14
  %321 = getelementptr inbounds i32, i32* %2, i64 8
  store i32 %320, i32* %321, align 4, !tbaa !14
  %322 = load i32, i32* %314, align 4, !tbaa !14
  %323 = getelementptr inbounds i32, i32* %2, i64 12
  store i32 %322, i32* %323, align 4, !tbaa !14
  %324 = load i32, i32* %313, align 4, !tbaa !14
  %325 = getelementptr inbounds i32, i32* %2, i64 64
  store i32 %324, i32* %325, align 4, !tbaa !14
  %326 = load i32, i32* %312, align 4, !tbaa !14
  %327 = getelementptr inbounds i32, i32* %2, i64 68
  store i32 %326, i32* %327, align 4, !tbaa !14
  %328 = load i32, i32* %311, align 4, !tbaa !14
  %329 = getelementptr inbounds i32, i32* %2, i64 72
  store i32 %328, i32* %329, align 4, !tbaa !14
  %330 = load i32, i32* %310, align 4, !tbaa !14
  %331 = getelementptr inbounds i32, i32* %2, i64 76
  store i32 %330, i32* %331, align 4, !tbaa !14
  %332 = load i32, i32* %309, align 4, !tbaa !14
  %333 = getelementptr inbounds i32, i32* %2, i64 128
  store i32 %332, i32* %333, align 4, !tbaa !14
  %334 = load i32, i32* %308, align 4, !tbaa !14
  %335 = getelementptr inbounds i32, i32* %2, i64 132
  store i32 %334, i32* %335, align 4, !tbaa !14
  %336 = load i32, i32* %307, align 4, !tbaa !14
  %337 = getelementptr inbounds i32, i32* %2, i64 136
  store i32 %336, i32* %337, align 4, !tbaa !14
  %338 = load i32, i32* %306, align 4, !tbaa !14
  %339 = getelementptr inbounds i32, i32* %2, i64 140
  store i32 %338, i32* %339, align 4, !tbaa !14
  %340 = load i32, i32* %305, align 4, !tbaa !14
  %341 = getelementptr inbounds i32, i32* %2, i64 192
  store i32 %340, i32* %341, align 4, !tbaa !14
  %342 = load i32, i32* %304, align 4, !tbaa !14
  %343 = getelementptr inbounds i32, i32* %2, i64 196
  store i32 %342, i32* %343, align 4, !tbaa !14
  %344 = load i32, i32* %303, align 4, !tbaa !14
  %345 = getelementptr inbounds i32, i32* %2, i64 200
  store i32 %344, i32* %345, align 4, !tbaa !14
  %346 = load i32, i32* %302, align 4, !tbaa !14
  %347 = getelementptr inbounds i32, i32* %2, i64 204
  store i32 %346, i32* %347, align 4, !tbaa !14
  %348 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 0, i32 0) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 0, i32 0) #3
  %349 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 0, i32 4) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 0, i32 4) #3
  %350 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 0, i32 8) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 0, i32 8) #3
  %351 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 0, i32 12) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 0, i32 12) #3
  %352 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 4, i32 0) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 4, i32 0) #3
  %353 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 4, i32 4) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 4, i32 4) #3
  %354 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 4, i32 8) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 4, i32 8) #3
  %355 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 4, i32 12) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 4, i32 12) #3
  %356 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 8, i32 0) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 8, i32 0) #3
  %357 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 8, i32 4) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 8, i32 4) #3
  %358 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 8, i32 8) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 8, i32 8) #3
  %359 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 8, i32 12) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 8, i32 12) #3
  %360 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 12, i32 0) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 12, i32 0) #3
  %361 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 12, i32 4) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 12, i32 4) #3
  %362 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 12, i32 8) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 12, i32 8) #3
  %363 = tail call zeroext i1 @quant_ac4x4_normal(i32* %2, i32 28, i32 12, i32 12) #3
  tail call void @inverse4x4(i32* %2, i32* nonnull %3, i32 12, i32 12) #3
  %364 = load i32, i32* %36, align 4, !tbaa !14
  store i32 %364, i32* %2, align 4, !tbaa !14
  %365 = getelementptr inbounds i8, i8* %35, i64 4
  %366 = bitcast i8* %365 to i32*
  %367 = load i32, i32* %366, align 4, !tbaa !14
  store i32 %367, i32* %319, align 4, !tbaa !14
  %368 = getelementptr inbounds i8, i8* %35, i64 8
  %369 = bitcast i8* %368 to i32*
  %370 = load i32, i32* %369, align 4, !tbaa !14
  store i32 %370, i32* %321, align 4, !tbaa !14
  %371 = getelementptr inbounds i8, i8* %35, i64 12
  %372 = bitcast i8* %371 to i32*
  %373 = load i32, i32* %372, align 4, !tbaa !14
  store i32 %373, i32* %323, align 4, !tbaa !14
  %374 = getelementptr inbounds i8, i8* %35, i64 16
  %375 = bitcast i8* %374 to i32*
  %376 = load i32, i32* %375, align 4, !tbaa !14
  store i32 %376, i32* %325, align 4, !tbaa !14
  %377 = getelementptr inbounds i8, i8* %35, i64 20
  %378 = bitcast i8* %377 to i32*
  %379 = load i32, i32* %378, align 4, !tbaa !14
  store i32 %379, i32* %327, align 4, !tbaa !14
  %380 = getelementptr inbounds i8, i8* %35, i64 24
  %381 = bitcast i8* %380 to i32*
  %382 = load i32, i32* %381, align 4, !tbaa !14
  store i32 %382, i32* %329, align 4, !tbaa !14
  %383 = getelementptr inbounds i8, i8* %35, i64 28
  %384 = bitcast i8* %383 to i32*
  %385 = load i32, i32* %384, align 4, !tbaa !14
  store i32 %385, i32* %331, align 4, !tbaa !14
  %386 = getelementptr inbounds i8, i8* %35, i64 32
  %387 = bitcast i8* %386 to i32*
  %388 = load i32, i32* %387, align 4, !tbaa !14
  store i32 %388, i32* %333, align 4, !tbaa !14
  %389 = getelementptr inbounds i8, i8* %35, i64 36
  %390 = bitcast i8* %389 to i32*
  %391 = load i32, i32* %390, align 4, !tbaa !14
  store i32 %391, i32* %335, align 4, !tbaa !14
  %392 = getelementptr inbounds i8, i8* %35, i64 40
  %393 = bitcast i8* %392 to i32*
  %394 = load i32, i32* %393, align 4, !tbaa !14
  store i32 %394, i32* %337, align 4, !tbaa !14
  %395 = getelementptr inbounds i8, i8* %35, i64 44
  %396 = bitcast i8* %395 to i32*
  %397 = load i32, i32* %396, align 4, !tbaa !14
  store i32 %397, i32* %339, align 4, !tbaa !14
  %398 = getelementptr inbounds i8, i8* %35, i64 48
  %399 = bitcast i8* %398 to i32*
  %400 = load i32, i32* %399, align 4, !tbaa !14
  store i32 %400, i32* %341, align 4, !tbaa !14
  %401 = getelementptr inbounds i8, i8* %35, i64 52
  %402 = bitcast i8* %401 to i32*
  %403 = load i32, i32* %402, align 4, !tbaa !14
  store i32 %403, i32* %343, align 4, !tbaa !14
  %404 = getelementptr inbounds i8, i8* %35, i64 56
  %405 = bitcast i8* %404 to i32*
  %406 = load i32, i32* %405, align 4, !tbaa !14
  store i32 %406, i32* %345, align 4, !tbaa !14
  %407 = getelementptr inbounds i8, i8* %35, i64 60
  %408 = bitcast i8* %407 to i32*
  %409 = load i32, i32* %408, align 4, !tbaa !14
  store i32 %409, i32* %347, align 4, !tbaa !14
  br label %411

; <label>:410:                                    ; preds = %411
  ret void

; <label>:411:                                    ; preds = %411, %301
  %412 = phi i64 [ 0, %301 ], [ %605, %411 ]
  %413 = shl i64 %412, 4
  %414 = getelementptr inbounds i32, i32* %3, i64 %413
  %415 = load i32, i32* %414, align 4, !tbaa !14
  %416 = add nsw i32 %415, 32
  %417 = ashr i32 %416, 6
  %418 = getelementptr inbounds i32, i32* %1, i64 %413
  %419 = load i32, i32* %418, align 4, !tbaa !14
  %420 = add nsw i32 %417, %419
  %421 = icmp sgt i32 %420, 0
  %422 = select i1 %421, i32 %420, i32 0
  %423 = icmp slt i32 %422, 255
  %424 = select i1 %423, i32 %422, i32 255
  store i32 %424, i32* %414, align 4, !tbaa !14
  %425 = or i64 %413, 1
  %426 = getelementptr inbounds i32, i32* %3, i64 %425
  %427 = load i32, i32* %426, align 4, !tbaa !14
  %428 = add nsw i32 %427, 32
  %429 = ashr i32 %428, 6
  %430 = getelementptr inbounds i32, i32* %1, i64 %425
  %431 = load i32, i32* %430, align 4, !tbaa !14
  %432 = add nsw i32 %429, %431
  %433 = icmp sgt i32 %432, 0
  %434 = select i1 %433, i32 %432, i32 0
  %435 = icmp slt i32 %434, 255
  %436 = select i1 %435, i32 %434, i32 255
  store i32 %436, i32* %426, align 4, !tbaa !14
  %437 = or i64 %413, 2
  %438 = getelementptr inbounds i32, i32* %3, i64 %437
  %439 = load i32, i32* %438, align 4, !tbaa !14
  %440 = add nsw i32 %439, 32
  %441 = ashr i32 %440, 6
  %442 = getelementptr inbounds i32, i32* %1, i64 %437
  %443 = load i32, i32* %442, align 4, !tbaa !14
  %444 = add nsw i32 %441, %443
  %445 = icmp sgt i32 %444, 0
  %446 = select i1 %445, i32 %444, i32 0
  %447 = icmp slt i32 %446, 255
  %448 = select i1 %447, i32 %446, i32 255
  store i32 %448, i32* %438, align 4, !tbaa !14
  %449 = or i64 %413, 3
  %450 = getelementptr inbounds i32, i32* %3, i64 %449
  %451 = load i32, i32* %450, align 4, !tbaa !14
  %452 = add nsw i32 %451, 32
  %453 = ashr i32 %452, 6
  %454 = getelementptr inbounds i32, i32* %1, i64 %449
  %455 = load i32, i32* %454, align 4, !tbaa !14
  %456 = add nsw i32 %453, %455
  %457 = icmp sgt i32 %456, 0
  %458 = select i1 %457, i32 %456, i32 0
  %459 = icmp slt i32 %458, 255
  %460 = select i1 %459, i32 %458, i32 255
  store i32 %460, i32* %450, align 4, !tbaa !14
  %461 = or i64 %413, 4
  %462 = getelementptr inbounds i32, i32* %3, i64 %461
  %463 = load i32, i32* %462, align 4, !tbaa !14
  %464 = add nsw i32 %463, 32
  %465 = ashr i32 %464, 6
  %466 = getelementptr inbounds i32, i32* %1, i64 %461
  %467 = load i32, i32* %466, align 4, !tbaa !14
  %468 = add nsw i32 %465, %467
  %469 = icmp sgt i32 %468, 0
  %470 = select i1 %469, i32 %468, i32 0
  %471 = icmp slt i32 %470, 255
  %472 = select i1 %471, i32 %470, i32 255
  store i32 %472, i32* %462, align 4, !tbaa !14
  %473 = or i64 %413, 5
  %474 = getelementptr inbounds i32, i32* %3, i64 %473
  %475 = load i32, i32* %474, align 4, !tbaa !14
  %476 = add nsw i32 %475, 32
  %477 = ashr i32 %476, 6
  %478 = getelementptr inbounds i32, i32* %1, i64 %473
  %479 = load i32, i32* %478, align 4, !tbaa !14
  %480 = add nsw i32 %477, %479
  %481 = icmp sgt i32 %480, 0
  %482 = select i1 %481, i32 %480, i32 0
  %483 = icmp slt i32 %482, 255
  %484 = select i1 %483, i32 %482, i32 255
  store i32 %484, i32* %474, align 4, !tbaa !14
  %485 = or i64 %413, 6
  %486 = getelementptr inbounds i32, i32* %3, i64 %485
  %487 = load i32, i32* %486, align 4, !tbaa !14
  %488 = add nsw i32 %487, 32
  %489 = ashr i32 %488, 6
  %490 = getelementptr inbounds i32, i32* %1, i64 %485
  %491 = load i32, i32* %490, align 4, !tbaa !14
  %492 = add nsw i32 %489, %491
  %493 = icmp sgt i32 %492, 0
  %494 = select i1 %493, i32 %492, i32 0
  %495 = icmp slt i32 %494, 255
  %496 = select i1 %495, i32 %494, i32 255
  store i32 %496, i32* %486, align 4, !tbaa !14
  %497 = or i64 %413, 7
  %498 = getelementptr inbounds i32, i32* %3, i64 %497
  %499 = load i32, i32* %498, align 4, !tbaa !14
  %500 = add nsw i32 %499, 32
  %501 = ashr i32 %500, 6
  %502 = getelementptr inbounds i32, i32* %1, i64 %497
  %503 = load i32, i32* %502, align 4, !tbaa !14
  %504 = add nsw i32 %501, %503
  %505 = icmp sgt i32 %504, 0
  %506 = select i1 %505, i32 %504, i32 0
  %507 = icmp slt i32 %506, 255
  %508 = select i1 %507, i32 %506, i32 255
  store i32 %508, i32* %498, align 4, !tbaa !14
  %509 = or i64 %413, 8
  %510 = getelementptr inbounds i32, i32* %3, i64 %509
  %511 = load i32, i32* %510, align 4, !tbaa !14
  %512 = add nsw i32 %511, 32
  %513 = ashr i32 %512, 6
  %514 = getelementptr inbounds i32, i32* %1, i64 %509
  %515 = load i32, i32* %514, align 4, !tbaa !14
  %516 = add nsw i32 %513, %515
  %517 = icmp sgt i32 %516, 0
  %518 = select i1 %517, i32 %516, i32 0
  %519 = icmp slt i32 %518, 255
  %520 = select i1 %519, i32 %518, i32 255
  store i32 %520, i32* %510, align 4, !tbaa !14
  %521 = or i64 %413, 9
  %522 = getelementptr inbounds i32, i32* %3, i64 %521
  %523 = load i32, i32* %522, align 4, !tbaa !14
  %524 = add nsw i32 %523, 32
  %525 = ashr i32 %524, 6
  %526 = getelementptr inbounds i32, i32* %1, i64 %521
  %527 = load i32, i32* %526, align 4, !tbaa !14
  %528 = add nsw i32 %525, %527
  %529 = icmp sgt i32 %528, 0
  %530 = select i1 %529, i32 %528, i32 0
  %531 = icmp slt i32 %530, 255
  %532 = select i1 %531, i32 %530, i32 255
  store i32 %532, i32* %522, align 4, !tbaa !14
  %533 = or i64 %413, 10
  %534 = getelementptr inbounds i32, i32* %3, i64 %533
  %535 = load i32, i32* %534, align 4, !tbaa !14
  %536 = add nsw i32 %535, 32
  %537 = ashr i32 %536, 6
  %538 = getelementptr inbounds i32, i32* %1, i64 %533
  %539 = load i32, i32* %538, align 4, !tbaa !14
  %540 = add nsw i32 %537, %539
  %541 = icmp sgt i32 %540, 0
  %542 = select i1 %541, i32 %540, i32 0
  %543 = icmp slt i32 %542, 255
  %544 = select i1 %543, i32 %542, i32 255
  store i32 %544, i32* %534, align 4, !tbaa !14
  %545 = or i64 %413, 11
  %546 = getelementptr inbounds i32, i32* %3, i64 %545
  %547 = load i32, i32* %546, align 4, !tbaa !14
  %548 = add nsw i32 %547, 32
  %549 = ashr i32 %548, 6
  %550 = getelementptr inbounds i32, i32* %1, i64 %545
  %551 = load i32, i32* %550, align 4, !tbaa !14
  %552 = add nsw i32 %549, %551
  %553 = icmp sgt i32 %552, 0
  %554 = select i1 %553, i32 %552, i32 0
  %555 = icmp slt i32 %554, 255
  %556 = select i1 %555, i32 %554, i32 255
  store i32 %556, i32* %546, align 4, !tbaa !14
  %557 = or i64 %413, 12
  %558 = getelementptr inbounds i32, i32* %3, i64 %557
  %559 = load i32, i32* %558, align 4, !tbaa !14
  %560 = add nsw i32 %559, 32
  %561 = ashr i32 %560, 6
  %562 = getelementptr inbounds i32, i32* %1, i64 %557
  %563 = load i32, i32* %562, align 4, !tbaa !14
  %564 = add nsw i32 %561, %563
  %565 = icmp sgt i32 %564, 0
  %566 = select i1 %565, i32 %564, i32 0
  %567 = icmp slt i32 %566, 255
  %568 = select i1 %567, i32 %566, i32 255
  store i32 %568, i32* %558, align 4, !tbaa !14
  %569 = or i64 %413, 13
  %570 = getelementptr inbounds i32, i32* %3, i64 %569
  %571 = load i32, i32* %570, align 4, !tbaa !14
  %572 = add nsw i32 %571, 32
  %573 = ashr i32 %572, 6
  %574 = getelementptr inbounds i32, i32* %1, i64 %569
  %575 = load i32, i32* %574, align 4, !tbaa !14
  %576 = add nsw i32 %573, %575
  %577 = icmp sgt i32 %576, 0
  %578 = select i1 %577, i32 %576, i32 0
  %579 = icmp slt i32 %578, 255
  %580 = select i1 %579, i32 %578, i32 255
  store i32 %580, i32* %570, align 4, !tbaa !14
  %581 = or i64 %413, 14
  %582 = getelementptr inbounds i32, i32* %3, i64 %581
  %583 = load i32, i32* %582, align 4, !tbaa !14
  %584 = add nsw i32 %583, 32
  %585 = ashr i32 %584, 6
  %586 = getelementptr inbounds i32, i32* %1, i64 %581
  %587 = load i32, i32* %586, align 4, !tbaa !14
  %588 = add nsw i32 %585, %587
  %589 = icmp sgt i32 %588, 0
  %590 = select i1 %589, i32 %588, i32 0
  %591 = icmp slt i32 %590, 255
  %592 = select i1 %591, i32 %590, i32 255
  store i32 %592, i32* %582, align 4, !tbaa !14
  %593 = or i64 %413, 15
  %594 = getelementptr inbounds i32, i32* %3, i64 %593
  %595 = load i32, i32* %594, align 4, !tbaa !14
  %596 = add nsw i32 %595, 32
  %597 = ashr i32 %596, 6
  %598 = getelementptr inbounds i32, i32* %1, i64 %593
  %599 = load i32, i32* %598, align 4, !tbaa !14
  %600 = add nsw i32 %597, %599
  %601 = icmp sgt i32 %600, 0
  %602 = select i1 %601, i32 %600, i32 0
  %603 = icmp slt i32 %602, 255
  %604 = select i1 %603, i32 %602, i32 255
  store i32 %604, i32* %594, align 4, !tbaa !14
  %605 = add nuw nsw i64 %412, 1
  %606 = icmp eq i64 %605, 16
  br i1 %606, label %410, label %411
}

declare i32 @sad(%struct.macroblock*, i32*) local_unnamed_addr #2

; Function Attrs: nounwind
declare void @free(i8* nocapture) local_unnamed_addr #1

declare void @forward4x4(i32*, i32*, i32, i32) local_unnamed_addr #2

declare void @hadamard4x4_DCs(i32*, i32*) local_unnamed_addr #2

declare zeroext i1 @quant_dc4x4_normal(i32*, i32) local_unnamed_addr #2

declare void @ihadamard4x4_DCs(i32*, i32*) local_unnamed_addr #2

declare void @inverse_quant_DCs(i32*, i32) local_unnamed_addr #2

declare zeroext i1 @quant_ac4x4_normal(i32*, i32, i32, i32) local_unnamed_addr #2

declare void @inverse4x4(i32*, i32*, i32, i32) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"}
!2 = !{!3, !7, i64 16}
!3 = !{!"macroblock", !4, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"int", !4, i64 0}
!7 = !{!"any pointer", !4, i64 0}
!8 = !{!3, !7, i64 24}
!9 = !{!3, !7, i64 32}
!10 = !{!3, !7, i64 40}
!11 = !{!3, !7, i64 48}
!12 = !{!13, !7, i64 24}
!13 = !{!"video", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12, !6, i64 16, !6, i64 20, !7, i64 24, !7, i64 32, !7, i64 40}
!14 = !{!6, !6, i64 0}
!15 = !{!3, !6, i64 4}
!16 = !{!13, !6, i64 4}
!17 = !{!13, !6, i64 0}
!18 = !{!3, !6, i64 8}
!19 = !{!3, !6, i64 12}
