; ModuleID = 'read_yuv.c'
source_filename = "read_yuv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.video = type { i32, i32, i32, i32, i32, i32, i32*, i32*, i32* }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"fd\00", align 1
@.str.2 = private unnamed_addr constant [11 x i8] c"read_yuv.c\00", align 1
@__PRETTY_FUNCTION__.ingest = private unnamed_addr constant [66 x i8] c"struct video *ingest(const char *, int, int, int, struct video *)\00", align 1
@.str.3 = private unnamed_addr constant [44 x i8] c"fread(&vid->y_data[start+w], 1, 1, fd) == 1\00", align 1
@.str.4 = private unnamed_addr constant [63 x i8] c"fread(&vid->u_data[i * height/2 * width/2 + r], 1, 1, fd) == 1\00", align 1
@.str.5 = private unnamed_addr constant [63 x i8] c"fread(&vid->v_data[i * height/2 * width/2 + r], 1, 1, fd) == 1\00", align 1

; Function Attrs: nounwind uwtable
define %struct.video* @ingest(i8* nocapture readonly, i32, i32, i32, %struct.video* returned) local_unnamed_addr #0 {
  %6 = tail call %struct._IO_FILE* @fopen(i8* %0, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0))
  %7 = icmp eq %struct._IO_FILE* %6, null
  br i1 %7, label %8, label %9

; <label>:8:                                      ; preds = %5
  tail call void @__assert_fail(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.1, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i64 0, i64 0), i32 9, i8* getelementptr inbounds ([66 x i8], [66 x i8]* @__PRETTY_FUNCTION__.ingest, i64 0, i64 0)) #4
  unreachable

; <label>:9:                                      ; preds = %5
  %10 = and i32 %1, 15
  %11 = icmp eq i32 %10, 0
  %12 = add nsw i32 %1, 16
  %13 = sub i32 %12, %10
  %14 = select i1 %11, i32 %1, i32 %13
  %15 = and i32 %2, 15
  %16 = icmp eq i32 %15, 0
  %17 = add nsw i32 %2, 16
  %18 = sub i32 %17, %15
  %19 = select i1 %16, i32 %2, i32 %18
  %20 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 0
  store i32 %14, i32* %20, align 8, !tbaa !2
  %21 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 1
  store i32 %19, i32* %21, align 4, !tbaa !8
  %22 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 3
  store i32 %3, i32* %22, align 4, !tbaa !9
  %23 = sdiv i32 %1, 2
  %24 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 4
  store i32 %23, i32* %24, align 8, !tbaa !10
  %25 = sdiv i32 %2, 2
  %26 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 5
  store i32 %25, i32* %26, align 4, !tbaa !11
  %27 = sext i32 %14 to i64
  %28 = sext i32 %19 to i64
  %29 = sext i32 %3 to i64
  %30 = shl nsw i64 %29, 2
  %31 = mul i64 %30, %27
  %32 = mul i64 %31, %28
  %33 = tail call noalias i8* @malloc(i64 %32) #5
  %34 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 6
  %35 = bitcast i32** %34 to i8**
  store i8* %33, i8** %35, align 8, !tbaa !12
  %36 = sext i32 %1 to i64
  %37 = shl nsw i64 %36, 1
  %38 = and i64 %37, 9223372036854775806
  %39 = sext i32 %2 to i64
  %40 = mul i64 %38, %39
  %41 = lshr exact i64 %40, 1
  %42 = mul i64 %41, %29
  %43 = tail call noalias i8* @malloc(i64 %42) #5
  %44 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 7
  %45 = bitcast i32** %44 to i8**
  store i8* %43, i8** %45, align 8, !tbaa !13
  %46 = tail call noalias i8* @malloc(i64 %42) #5
  %47 = getelementptr inbounds %struct.video, %struct.video* %4, i64 0, i32 8
  %48 = bitcast i32** %47 to i8**
  store i8* %46, i8** %48, align 8, !tbaa !14
  %49 = icmp sgt i32 %3, 0
  br i1 %49, label %50, label %80

; <label>:50:                                     ; preds = %9
  %51 = icmp sgt i32 %1, 0
  %52 = icmp sgt i32 %14, %1
  %53 = icmp sgt i32 %2, 0
  %54 = icmp sgt i32 %19, %2
  %55 = add i32 %2, -1
  %56 = mul nsw i32 %23, %2
  %57 = sdiv i32 %56, 2
  %58 = icmp sgt i32 %56, 1
  %59 = add nsw i32 %1, -1
  %60 = mul nsw i32 %19, %59
  %61 = shl nsw i64 %28, 2
  %62 = sext i32 %57 to i64
  %63 = sext i32 %60 to i64
  %64 = zext i32 %1 to i64
  %65 = sub nsw i64 %28, %39
  %66 = add nsw i64 %28, -1
  %67 = sub nsw i64 %66, %39
  %68 = sub nsw i64 %27, %36
  %69 = add nsw i64 %27, -1
  %70 = sub nsw i64 %69, %36
  %71 = and i64 %65, 3
  %72 = icmp eq i64 %71, 0
  %73 = icmp ult i64 %67, 3
  %74 = and i64 %68, 3
  %75 = icmp eq i64 %74, 0
  %76 = and i64 %65, 3
  %77 = icmp eq i64 %76, 0
  %78 = icmp ult i64 %67, 3
  %79 = icmp ult i64 %70, 3
  br label %82

; <label>:80:                                     ; preds = %292, %9
  %81 = tail call i32 @fclose(%struct._IO_FILE* nonnull %6)
  ret %struct.video* %4

; <label>:82:                                     ; preds = %50, %292
  %83 = phi i64 [ 0, %50 ], [ %293, %292 ]
  br i1 %51, label %84, label %194

; <label>:84:                                     ; preds = %82
  %85 = mul nsw i64 %83, %27
  br i1 %53, label %86, label %146

; <label>:86:                                     ; preds = %84
  %87 = trunc i64 %85 to i32
  br label %88

; <label>:88:                                     ; preds = %96, %86
  %89 = phi i64 [ %97, %96 ], [ 0, %86 ]
  %90 = trunc i64 %89 to i32
  %91 = add i32 %90, %87
  %92 = mul i32 %91, %19
  %93 = sext i32 %92 to i64
  br label %118

; <label>:94:                                     ; preds = %118
  %95 = icmp slt i64 %126, %39
  br i1 %95, label %118, label %127

; <label>:96:                                     ; preds = %143, %99, %127
  %97 = add nuw nsw i64 %89, 1
  %98 = icmp slt i64 %97, %36
  br i1 %98, label %88, label %194

; <label>:99:                                     ; preds = %99, %145
  %100 = phi i64 [ %144, %145 ], [ %116, %99 ]
  %101 = load i32, i32* %132, align 4, !tbaa !15
  %102 = add nsw i64 %100, %93
  %103 = getelementptr inbounds i32, i32* %129, i64 %102
  store i32 %101, i32* %103, align 4, !tbaa !15
  %104 = add nsw i64 %100, 1
  %105 = load i32, i32* %132, align 4, !tbaa !15
  %106 = add nsw i64 %104, %93
  %107 = getelementptr inbounds i32, i32* %129, i64 %106
  store i32 %105, i32* %107, align 4, !tbaa !15
  %108 = add nsw i64 %100, 2
  %109 = load i32, i32* %132, align 4, !tbaa !15
  %110 = add nsw i64 %108, %93
  %111 = getelementptr inbounds i32, i32* %129, i64 %110
  store i32 %109, i32* %111, align 4, !tbaa !15
  %112 = add nsw i64 %100, 3
  %113 = load i32, i32* %132, align 4, !tbaa !15
  %114 = add nsw i64 %112, %93
  %115 = getelementptr inbounds i32, i32* %129, i64 %114
  store i32 %113, i32* %115, align 4, !tbaa !15
  %116 = add nsw i64 %100, 4
  %117 = icmp eq i64 %116, %28
  br i1 %117, label %96, label %99

; <label>:118:                                    ; preds = %88, %94
  %119 = phi i64 [ 0, %88 ], [ %126, %94 ]
  %120 = load i32*, i32** %34, align 8, !tbaa !12
  %121 = add nsw i64 %119, %93
  %122 = getelementptr inbounds i32, i32* %120, i64 %121
  %123 = bitcast i32* %122 to i8*
  %124 = tail call i64 @fread(i8* %123, i64 1, i64 1, %struct._IO_FILE* nonnull %6)
  %125 = icmp eq i64 %124, 1
  %126 = add nuw nsw i64 %119, 1
  br i1 %125, label %94, label %216

; <label>:127:                                    ; preds = %94
  br i1 %54, label %128, label %96

; <label>:128:                                    ; preds = %127
  %129 = load i32*, i32** %34, align 8, !tbaa !12
  %130 = add i32 %55, %92
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds i32, i32* %129, i64 %131
  br i1 %72, label %143, label %133

; <label>:133:                                    ; preds = %128
  br label %134

; <label>:134:                                    ; preds = %134, %133
  %135 = phi i64 [ %39, %133 ], [ %140, %134 ]
  %136 = phi i64 [ %71, %133 ], [ %141, %134 ]
  %137 = load i32, i32* %132, align 4, !tbaa !15
  %138 = add nsw i64 %135, %93
  %139 = getelementptr inbounds i32, i32* %129, i64 %138
  store i32 %137, i32* %139, align 4, !tbaa !15
  %140 = add nsw i64 %135, 1
  %141 = add i64 %136, -1
  %142 = icmp eq i64 %141, 0
  br i1 %142, label %143, label %134, !llvm.loop !16

; <label>:143:                                    ; preds = %134, %128
  %144 = phi i64 [ %39, %128 ], [ %140, %134 ]
  br i1 %73, label %96, label %145

; <label>:145:                                    ; preds = %143
  br label %99

; <label>:146:                                    ; preds = %84
  br i1 %54, label %147, label %194

; <label>:147:                                    ; preds = %146
  %148 = load i32*, i32** %34, align 8, !tbaa !12
  %149 = trunc i64 %85 to i32
  br label %150

; <label>:150:                                    ; preds = %191, %147
  %151 = phi i64 [ %192, %191 ], [ 0, %147 ]
  %152 = trunc i64 %151 to i32
  %153 = add i32 %152, %149
  %154 = mul i32 %153, %19
  %155 = add i32 %55, %154
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds i32, i32* %148, i64 %156
  %158 = sext i32 %154 to i64
  br i1 %77, label %169, label %159

; <label>:159:                                    ; preds = %150
  br label %160

; <label>:160:                                    ; preds = %160, %159
  %161 = phi i64 [ %39, %159 ], [ %166, %160 ]
  %162 = phi i64 [ %76, %159 ], [ %167, %160 ]
  %163 = load i32, i32* %157, align 4, !tbaa !15
  %164 = add nsw i64 %161, %158
  %165 = getelementptr inbounds i32, i32* %148, i64 %164
  store i32 %163, i32* %165, align 4, !tbaa !15
  %166 = add nsw i64 %161, 1
  %167 = add i64 %162, -1
  %168 = icmp eq i64 %167, 0
  br i1 %168, label %169, label %160, !llvm.loop !18

; <label>:169:                                    ; preds = %160, %150
  %170 = phi i64 [ %39, %150 ], [ %166, %160 ]
  br i1 %78, label %191, label %171

; <label>:171:                                    ; preds = %169
  br label %172

; <label>:172:                                    ; preds = %172, %171
  %173 = phi i64 [ %170, %171 ], [ %189, %172 ]
  %174 = load i32, i32* %157, align 4, !tbaa !15
  %175 = add nsw i64 %173, %158
  %176 = getelementptr inbounds i32, i32* %148, i64 %175
  store i32 %174, i32* %176, align 4, !tbaa !15
  %177 = add nsw i64 %173, 1
  %178 = load i32, i32* %157, align 4, !tbaa !15
  %179 = add nsw i64 %177, %158
  %180 = getelementptr inbounds i32, i32* %148, i64 %179
  store i32 %178, i32* %180, align 4, !tbaa !15
  %181 = add nsw i64 %173, 2
  %182 = load i32, i32* %157, align 4, !tbaa !15
  %183 = add nsw i64 %181, %158
  %184 = getelementptr inbounds i32, i32* %148, i64 %183
  store i32 %182, i32* %184, align 4, !tbaa !15
  %185 = add nsw i64 %173, 3
  %186 = load i32, i32* %157, align 4, !tbaa !15
  %187 = add nsw i64 %185, %158
  %188 = getelementptr inbounds i32, i32* %148, i64 %187
  store i32 %186, i32* %188, align 4, !tbaa !15
  %189 = add nsw i64 %173, 4
  %190 = icmp eq i64 %189, %28
  br i1 %190, label %191, label %172

; <label>:191:                                    ; preds = %172, %169
  %192 = add nuw nsw i64 %151, 1
  %193 = icmp eq i64 %192, %64
  br i1 %193, label %194, label %150

; <label>:194:                                    ; preds = %191, %96, %146, %82
  br i1 %52, label %195, label %217

; <label>:195:                                    ; preds = %194
  %196 = mul nsw i64 %83, %27
  %197 = mul nsw i64 %196, %28
  %198 = add nsw i64 %197, %63
  br i1 %75, label %213, label %199

; <label>:199:                                    ; preds = %195
  br label %200

; <label>:200:                                    ; preds = %200, %199
  %201 = phi i64 [ %36, %199 ], [ %210, %200 ]
  %202 = phi i64 [ %74, %199 ], [ %211, %200 ]
  %203 = mul nsw i64 %201, %28
  %204 = add nsw i64 %203, %197
  %205 = load i32*, i32** %34, align 8, !tbaa !12
  %206 = getelementptr inbounds i32, i32* %205, i64 %204
  %207 = bitcast i32* %206 to i8*
  %208 = getelementptr inbounds i32, i32* %205, i64 %198
  %209 = bitcast i32* %208 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %207, i8* %209, i64 %61, i32 4, i1 false)
  %210 = add nsw i64 %201, 1
  %211 = add i64 %202, -1
  %212 = icmp eq i64 %211, 0
  br i1 %212, label %213, label %200, !llvm.loop !19

; <label>:213:                                    ; preds = %200, %195
  %214 = phi i64 [ %36, %195 ], [ %210, %200 ]
  br i1 %79, label %217, label %215

; <label>:215:                                    ; preds = %213
  br label %225

; <label>:216:                                    ; preds = %118
  tail call void @__assert_fail(i8* getelementptr inbounds ([44 x i8], [44 x i8]* @.str.3, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i64 0, i64 0), i32 30, i8* getelementptr inbounds ([66 x i8], [66 x i8]* @__PRETTY_FUNCTION__.ingest, i64 0, i64 0)) #4
  unreachable

; <label>:217:                                    ; preds = %213, %225, %194
  br i1 %58, label %218, label %292

; <label>:218:                                    ; preds = %217
  %219 = trunc i64 %83 to i32
  %220 = mul i32 %219, %1
  %221 = sdiv i32 %220, 2
  %222 = mul nsw i32 %221, %2
  %223 = sdiv i32 %222, 2
  %224 = sext i32 %223 to i64
  br label %262

; <label>:225:                                    ; preds = %225, %215
  %226 = phi i64 [ %214, %215 ], [ %258, %225 ]
  %227 = mul nsw i64 %226, %28
  %228 = add nsw i64 %227, %197
  %229 = load i32*, i32** %34, align 8, !tbaa !12
  %230 = getelementptr inbounds i32, i32* %229, i64 %228
  %231 = bitcast i32* %230 to i8*
  %232 = getelementptr inbounds i32, i32* %229, i64 %198
  %233 = bitcast i32* %232 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %231, i8* %233, i64 %61, i32 4, i1 false)
  %234 = add nsw i64 %226, 1
  %235 = mul nsw i64 %234, %28
  %236 = add nsw i64 %235, %197
  %237 = load i32*, i32** %34, align 8, !tbaa !12
  %238 = getelementptr inbounds i32, i32* %237, i64 %236
  %239 = bitcast i32* %238 to i8*
  %240 = getelementptr inbounds i32, i32* %237, i64 %198
  %241 = bitcast i32* %240 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %239, i8* %241, i64 %61, i32 4, i1 false)
  %242 = add nsw i64 %226, 2
  %243 = mul nsw i64 %242, %28
  %244 = add nsw i64 %243, %197
  %245 = load i32*, i32** %34, align 8, !tbaa !12
  %246 = getelementptr inbounds i32, i32* %245, i64 %244
  %247 = bitcast i32* %246 to i8*
  %248 = getelementptr inbounds i32, i32* %245, i64 %198
  %249 = bitcast i32* %248 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %247, i8* %249, i64 %61, i32 4, i1 false)
  %250 = add nsw i64 %226, 3
  %251 = mul nsw i64 %250, %28
  %252 = add nsw i64 %251, %197
  %253 = load i32*, i32** %34, align 8, !tbaa !12
  %254 = getelementptr inbounds i32, i32* %253, i64 %252
  %255 = bitcast i32* %254 to i8*
  %256 = getelementptr inbounds i32, i32* %253, i64 %198
  %257 = bitcast i32* %256 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %255, i8* %257, i64 %61, i32 4, i1 false)
  %258 = add nsw i64 %226, 4
  %259 = icmp eq i64 %258, %27
  br i1 %259, label %217, label %225

; <label>:260:                                    ; preds = %262
  %261 = icmp slt i64 %270, %62
  br i1 %261, label %262, label %272

; <label>:262:                                    ; preds = %218, %260
  %263 = phi i64 [ 0, %218 ], [ %270, %260 ]
  %264 = load i32*, i32** %44, align 8, !tbaa !13
  %265 = add nsw i64 %263, %224
  %266 = getelementptr inbounds i32, i32* %264, i64 %265
  %267 = bitcast i32* %266 to i8*
  %268 = tail call i64 @fread(i8* %267, i64 1, i64 1, %struct._IO_FILE* nonnull %6)
  %269 = icmp eq i64 %268, 1
  %270 = add nuw nsw i64 %263, 1
  br i1 %269, label %260, label %271

; <label>:271:                                    ; preds = %262
  tail call void @__assert_fail(i8* getelementptr inbounds ([63 x i8], [63 x i8]* @.str.4, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i64 0, i64 0), i32 46, i8* getelementptr inbounds ([66 x i8], [66 x i8]* @__PRETTY_FUNCTION__.ingest, i64 0, i64 0)) #4
  unreachable

; <label>:272:                                    ; preds = %260
  br i1 %58, label %273, label %292

; <label>:273:                                    ; preds = %272
  %274 = trunc i64 %83 to i32
  %275 = mul i32 %274, %1
  %276 = sdiv i32 %275, 2
  %277 = mul nsw i32 %276, %2
  %278 = sdiv i32 %277, 2
  %279 = sext i32 %278 to i64
  br label %282

; <label>:280:                                    ; preds = %282
  %281 = icmp slt i64 %290, %62
  br i1 %281, label %282, label %292

; <label>:282:                                    ; preds = %273, %280
  %283 = phi i64 [ 0, %273 ], [ %290, %280 ]
  %284 = load i32*, i32** %47, align 8, !tbaa !14
  %285 = add nsw i64 %283, %279
  %286 = getelementptr inbounds i32, i32* %284, i64 %285
  %287 = bitcast i32* %286 to i8*
  %288 = tail call i64 @fread(i8* %287, i64 1, i64 1, %struct._IO_FILE* nonnull %6)
  %289 = icmp eq i64 %288, 1
  %290 = add nuw nsw i64 %283, 1
  br i1 %289, label %280, label %291

; <label>:291:                                    ; preds = %282
  tail call void @__assert_fail(i8* getelementptr inbounds ([63 x i8], [63 x i8]* @.str.5, i64 0, i64 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.2, i64 0, i64 0), i32 50, i8* getelementptr inbounds ([66 x i8], [66 x i8]* @__PRETTY_FUNCTION__.ingest, i64 0, i64 0)) #4
  unreachable

; <label>:292:                                    ; preds = %280, %217, %272
  %293 = add nuw nsw i64 %83, 1
  %294 = icmp slt i64 %293, %29
  br i1 %294, label %82, label %80
}

; Function Attrs: nounwind
declare noalias %struct._IO_FILE* @fopen(i8* nocapture readonly, i8* nocapture readonly) local_unnamed_addr #1

; Function Attrs: noreturn nounwind
declare void @__assert_fail(i8*, i8*, i32, i8*) local_unnamed_addr #2

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) local_unnamed_addr #1

; Function Attrs: nounwind
declare i64 @fread(i8* nocapture, i64, i64, %struct._IO_FILE* nocapture) local_unnamed_addr #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #3

; Function Attrs: nounwind
declare i32 @fclose(%struct._IO_FILE* nocapture) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { noreturn nounwind }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"video", !4, i64 0, !4, i64 4, !4, i64 8, !4, i64 12, !4, i64 16, !4, i64 20, !7, i64 24, !7, i64 32, !7, i64 40}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"any pointer", !5, i64 0}
!8 = !{!3, !4, i64 4}
!9 = !{!3, !4, i64 12}
!10 = !{!3, !4, i64 16}
!11 = !{!3, !4, i64 20}
!12 = !{!3, !7, i64 24}
!13 = !{!3, !7, i64 32}
!14 = !{!3, !7, i64 40}
!15 = !{!4, !4, i64 0}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.unroll.disable"}
!18 = distinct !{!18, !17}
!19 = distinct !{!19, !17}
