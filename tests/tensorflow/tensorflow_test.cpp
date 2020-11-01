#include <gtest/gtest.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/delegates/gpu/delegate.h>


class TensorflowTest : public testing::Test {
};

TEST(TensorflowTest, tensorflow_lite_test_1) {
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("/test-space/linear.tflite");

    if(!model){
        printf("Failed to mmap model\n");
        exit(1);
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    std::unique_ptr<tflite::Interpreter> interpreter;
    tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);

    auto *delegate = TfLiteGpuDelegateV2Create(nullptr);
    ASSERT_EQ(interpreter->ModifyGraphWithDelegate(delegate), kTfLiteOk);
    // Resize input tensors, if desired.
    interpreter->AllocateTensors();

    float* input = interpreter->typed_input_tensor<float>(0);
    // Dummy input for testing
    *input = 2.0;

    interpreter->Invoke();

    float* output = interpreter->typed_output_tensor<float>(0);

    TfLiteGpuDelegateV2Delete(delegate);

    printf("Result is: %f\n", *output);
}