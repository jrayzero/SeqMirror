; ModuleID = 'main.c'
source_filename = "main.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.timespec = type { i64, i64 }
%struct.video = type { i32, i32, i32, i32, i32, i32, i32*, i32*, i32* }

@stderr = external local_unnamed_addr global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [16 x i8] c"Elapsed: %luns\0A\00", align 1
@.str.1 = private unnamed_addr constant [28 x i8] c"foreman_part_qcif_x1000.yuv\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"Iter %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define { i64, i64 } @ctick() local_unnamed_addr #0 {
  %1 = alloca %struct.timespec, align 8
  %2 = bitcast %struct.timespec* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %2) #4
  %3 = call i32 @clock_gettime(i32 1, %struct.timespec* nonnull %1) #4
  %4 = getelementptr inbounds %struct.timespec, %struct.timespec* %1, i64 0, i32 0
  %5 = load i64, i64* %4, align 8
  %6 = getelementptr inbounds %struct.timespec, %struct.timespec* %1, i64 0, i32 1
  %7 = load i64, i64* %6, align 8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %2) #4
  %8 = insertvalue { i64, i64 } undef, i64 %5, 0
  %9 = insertvalue { i64, i64 } %8, i64 %7, 1
  ret { i64, i64 } %9
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare i32 @clock_gettime(i32, %struct.timespec*) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define { i64, i64 } @ctock(i64, i64, i32) local_unnamed_addr #0 {
  %4 = alloca %struct.timespec, align 8
  %5 = bitcast %struct.timespec* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %5) #4
  %6 = call i32 @clock_gettime(i32 1, %struct.timespec* nonnull %4) #4
  %7 = getelementptr inbounds %struct.timespec, %struct.timespec* %4, i64 0, i32 0
  %8 = load i64, i64* %7, align 8, !tbaa !2
  %9 = sub nsw i64 %8, %0
  %10 = getelementptr inbounds %struct.timespec, %struct.timespec* %4, i64 0, i32 1
  %11 = load i64, i64* %10, align 8, !tbaa !7
  %12 = sub i64 %11, %1
  %13 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %14 = mul i64 %9, 1000000000
  %15 = add i64 %12, %14
  %16 = sext i32 %2 to i64
  %17 = udiv i64 %15, %16
  %18 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %13, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i64 %17) #5
  %19 = load i64, i64* %7, align 8
  %20 = load i64, i64* %10, align 8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %5) #4
  %21 = insertvalue { i64, i64 } undef, i64 %19, 0
  %22 = insertvalue { i64, i64 } %21, i64 %20, 1
  ret { i64, i64 } %22
}

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
  %1 = alloca <2 x i64>, align 16
  %2 = bitcast <2 x i64>* %1 to %struct.timespec*
  %3 = alloca <2 x i64>, align 16
  %4 = bitcast <2 x i64>* %3 to %struct.timespec*
  %5 = bitcast <2 x i64>* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %5) #4
  %6 = call i32 @clock_gettime(i32 1, %struct.timespec* nonnull %4) #4
  %7 = load <2 x i64>, <2 x i64>* %3, align 16
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %5) #4
  %8 = call noalias i8* @malloc(i64 48) #4
  %9 = bitcast i8* %8 to %struct.video*
  %10 = getelementptr inbounds i8, i8* %8, i64 24
  %11 = bitcast i8* %10 to i8**
  %12 = getelementptr inbounds i8, i8* %8, i64 32
  %13 = bitcast i8* %12 to i8**
  %14 = getelementptr inbounds i8, i8* %8, i64 40
  %15 = bitcast i8* %14 to i8**
  %16 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %17 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %16, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 0) #5
  %18 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %19 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %19) #4
  %20 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %20) #4
  %21 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %21) #4
  %22 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %23 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %22, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 1) #5
  %24 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %25 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %25) #4
  %26 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %26) #4
  %27 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %27) #4
  %28 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %29 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %28, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 2) #5
  %30 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %31 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %31) #4
  %32 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %32) #4
  %33 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %33) #4
  %34 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %35 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %34, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 3) #5
  %36 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %37 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %37) #4
  %38 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %38) #4
  %39 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %39) #4
  %40 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %41 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %40, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 4) #5
  %42 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %43 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %43) #4
  %44 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %44) #4
  %45 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %45) #4
  %46 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %47 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %46, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 5) #5
  %48 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %49 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %49) #4
  %50 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %50) #4
  %51 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %51) #4
  %52 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %53 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %52, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 6) #5
  %54 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %55 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %55) #4
  %56 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %56) #4
  %57 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %57) #4
  %58 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %59 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %58, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 7) #5
  %60 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %61 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %61) #4
  %62 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %62) #4
  %63 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %63) #4
  %64 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %65 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %64, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 8) #5
  %66 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %67 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %67) #4
  %68 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %68) #4
  %69 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %69) #4
  %70 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %71 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %70, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i64 0, i64 0), i32 9) #5
  %72 = call %struct.video* @ingest(i8* getelementptr inbounds ([28 x i8], [28 x i8]* @.str.1, i64 0, i64 0), i32 144, i32 176, i32 1000, %struct.video* %9) #4
  %73 = load i8*, i8** %11, align 8, !tbaa !10
  call void @free(i8* %73) #4
  %74 = load i8*, i8** %13, align 8, !tbaa !13
  call void @free(i8* %74) #4
  %75 = load i8*, i8** %15, align 8, !tbaa !14
  call void @free(i8* %75) #4
  %76 = bitcast <2 x i64>* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %76) #4
  %77 = call i32 @clock_gettime(i32 1, %struct.timespec* nonnull %2) #4
  %78 = load <2 x i64>, <2 x i64>* %1, align 16, !tbaa !15
  %79 = sub <2 x i64> %78, %7
  %80 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !8
  %81 = extractelement <2 x i64> %79, i32 0
  %82 = mul i64 %81, 1000000000
  %83 = extractelement <2 x i64> %79, i32 1
  %84 = add i64 %83, %82
  %85 = udiv i64 %84, 10
  %86 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %80, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i64 %85) #6
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %76) #4
  ret i32 0
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) local_unnamed_addr #2

declare %struct.video* @ingest(i8*, i32, i32, i32, %struct.video*) local_unnamed_addr #3

; Function Attrs: nounwind
declare void @free(i8* nocapture) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { cold }
attributes #6 = { cold nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"timespec", !4, i64 0, !4, i64 8}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!3, !4, i64 8}
!8 = !{!9, !9, i64 0}
!9 = !{!"any pointer", !5, i64 0}
!10 = !{!11, !9, i64 24}
!11 = !{!"video", !12, i64 0, !12, i64 4, !12, i64 8, !12, i64 12, !12, i64 16, !12, i64 20, !9, i64 24, !9, i64 32, !9, i64 40}
!12 = !{!"int", !5, i64 0}
!13 = !{!11, !9, i64 32}
!14 = !{!11, !9, i64 40}
!15 = !{!4, !4, i64 0}
