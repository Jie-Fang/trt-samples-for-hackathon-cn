/*
 * Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cuda_fp16.h"

#include <NvInfer.h>


#define CEIL(x, y) (((x) + (y)-1) / (y))

using namespace nvinfer1;

class WherePlugin : public IPluginV2Ext
{
private:
    struct
    {
        int nElement;
    } m;

public:
    int getNbOutputs() const override
    {
        return 1;
    }

    DataType getOutputDataType(int index, const DataType *inputTypes, int nbInputs) const
    {
        return inputTypes[1];
    }

    Dims getOutputDimensions(int index, const Dims *pInputDim, int nInputDim) override
    {
        return pInputDim[0];
    }

    bool supportsFormat(DataType type, PluginFormat format) const override
    {
        return (type == DataType::kFLOAT || type == DataType::kINT32) && format == PluginFormat::kNCHW;
    }

    size_t getWorkspaceSize(int nBatch) const override
    {
        return 0;
    }

    void configurePlugin(const Dims *inputDims, int nbInputs, const Dims *outputDims, int nbOutputs, const DataType *inputTypes, const DataType *outputTypes, const bool *inputIsBroadcast, const bool *outputIsBroadcast, PluginFormat floatFormat, int maxBatchSize)
    {
        m.nElement = inputDims[0].d[0];
        if (inputDims[0].nbDims == 2)
            m.nElement = m.nElement * inputDims[0].d[1];
    }

    int enqueue(int nBatch, const void *const *inputs, void **outputs, void *workspace, cudaStream_t stream) override;
    WherePlugin() {}
    WherePlugin(const void *buffer, size_t length)
    {
        memcpy(&m, buffer, sizeof(m));
    }

    IPluginV2Ext *clone() const override
    {
        return new WherePlugin(&m, sizeof(m));
    }
    int initialize() override
    {
        return 0;
    }
    void terminate() override {}
    void destroy() override
    {
        delete this;
    }
    bool isOutputBroadcastAcrossBatch(int outputIndex, const bool *inputIsBroadcasted, int nbInputs) const
    {
        return false;
    }
    bool canBroadcastInputAcrossBatch(int inputIndex) const
    {
        return false;
    }
    void        attachToContext(cudnnContext *contextCudnn, cublasContext *contextCublas, IGpuAllocator *gpuAllocator) {}
    void        detachFromContext() {}
    const char *getPluginType() const override
    {
        return "WherePlugin";
    }
    const char *getPluginVersion() const override
    {
        return "0";
    }
    void        setPluginNamespace(const char *szNamespace) override {}
    const char *getPluginNamespace() const override
    {
        return "";
    }
    virtual size_t getSerializationSize() const override
    {
        return sizeof(m);
    }
    virtual void serialize(void *buffer) const override
    {
        memcpy(buffer, &m, sizeof(m));
    }
};

class WherePluginCreator : public IPluginCreator
{
public:
    IPluginV2 *createPlugin(const char *name, const PluginFieldCollection *fc) override
    {
        return new WherePlugin();
    }

    IPluginV2 *deserializePlugin(const char *name, const void *serialData, size_t serialLength) override
    {
        return new WherePlugin(serialData, serialLength);
    }
    const char *getPluginName() const override
    {
        return "WherePlugin";
    }
    const char *getPluginVersion() const override
    {
        return "0";
    }
    void        setPluginNamespace(const char *szNamespace) override {}
    const char *getPluginNamespace() const override
    {
        return "";
    }
    const PluginFieldCollection *getFieldNames() override
    {
        return nullptr;
    }
};
