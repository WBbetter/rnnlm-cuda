#include "rnnlm-russian-morphology.h"

namespace RNNLM
{
void RnnlmRussianMorphology::computeNet(int last_word, int last_morph, int word)
{
    computeRecurrentLayer_(last_word, last_morph);
    if (word!=-1)
    {
        computeOutputLayer_();
        incremetLogProbByWordLP_(word, last_morph);
    }
}

void RnnlmRussianMorphology::learnNet(int last_word, int word, double alpha, double beta, int counter)
{
//    if (bptt>0) updateBptt_(last_word); //shift bptt layers

//    double beta2 = beta*alpha;

//    if (word==-1) return;

//    computeErrorOnOutput_(word);

//    computeErrorOnPrevious_(neu2, syn1, neu1);

//    if ((counter%10)==0)
//        applyGradient_(alpha, neu2.er, neu1.ac, syn1, beta2);
//    else
//        applyGradient_(alpha, neu2.er, neu1.ac, syn1, 0);

//  //  syn1.print();

//    if (bptt<=1)
//    {
//        neu1.softmaxErrorActivation();

//        //weight update 1->0
//        if (last_word!=-1)
//        {
//            if ((counter%10)==0)
//                applyGradient_(alpha, neu1.er, last_word, syn0v, beta2);
//            else
//                applyGradient_(alpha, neu1.er, last_word, syn0v, 0);
//        }

//        if ((counter%10)==0)
//            applyGradient_(alpha,neu1.er,neu0.ac,syn0h,beta2);
//        else
//            applyGradient_(alpha,neu1.er,neu0.ac,syn0h,0);
//    }
//    else		//BPTT
//    {
//        makeBptt_(word, alpha, beta, counter);
//    }
}

void RnnlmRussianMorphology::copyHiddenLayerToInput()
{
    neu0.ac = neu1.ac;
}

void RnnlmRussianMorphology::saveWeights()      //saves current weights and unit activations
{
//    neu0b = neu0;
//    neu1b = neu1;
//    neu2b = neu2;
//    syn0vb = syn0v;
//    syn0hb = syn0h;
//    syn1b = syn1;
}

void RnnlmRussianMorphology::restoreWeights()      //restores current weights and unit activations from backup copy
{
//    neu0 = neu0b;
//    neu1 = neu1b;
//    neu2 = neu2b;
//    syn0v = syn0vb;
//    syn0h = syn0hb;
//    syn1 = syn1b;
}

void RnnlmRussianMorphology::saveContext()		//useful for n-best list processing
{
    neu1b.ac = neu1.ac;
}

void RnnlmRussianMorphology::restoreContext()
{
    neu1.ac = neu1b.ac;
}

void RnnlmRussianMorphology::initNet(int i_vocabSize, const RusModelOptions& i_options)
{
    layer1_size = i_options.layer1_size;
    bptt = i_options.bptt;
    bptt_block = i_options.bptt_block;
    m_independent = i_options.independent;
    m_vocabSize = i_vocabSize;
    initNet_();
}

void RnnlmRussianMorphology::initNet_()
{
    neu0.setConstant(layer1_size, 0);
    neu1.setConstant(layer1_size, 0);
    neu2v.setConstant(m_vocabSize, 0);
    neu2m.setConstant(m_morphologySize, 0);

    neu0b.setConstant(layer1_size, 0);
    neu1b.setConstant(layer1_size, 0);
    neu2b.setConstant(m_vocabSize, 0);

    syn0vb.setZero(m_vocabSize, layer1_size);
    syn0mb.setZero(m_morphologySize, layer1_size);
    syn0hb.setZero(layer1_size,layer1_size);

    syn1vb.setZero(layer1_size, m_vocabSize);
    syn1vb.setZero(layer1_size, m_morphologySize);

    double* syn1v_init = (double *)calloc(layer1_size*(m_vocabSize+1), sizeof(double));
    double* syn1m_init = (double *)calloc(layer1_size*(m_morphologySize+1), sizeof(double));
    double* syn0v_init=(double *)calloc(layer1_size*m_vocabSize, sizeof(double));
    double* syn0m_init=(double *)calloc(layer1_size*m_morphologySize, sizeof(double));
    double* syn0h_init=(double *)calloc(layer1_size*layer1_size, sizeof(double));

    for (int b=0; b<m_vocabSize+1; b++) for (int a=0; a<layer1_size; a++)
    {
        syn1v_init[a+b*layer1_size]=random(-0.1, 0.1)+random(-0.1, 0.1)+random(-0.1, 0.1);
    }

    for (int b=0; b<m_morphologySize+1; b++) for (int a=0; a<layer1_size; a++)
    {
        syn1m_init[a+b*layer1_size]=random(-0.1, 0.1)+random(-0.1, 0.1)+random(-0.1, 0.1);
    }

    for(int i = 0; i < layer1_size; i++)
    {
        for(int j = 0; j < m_vocabSize; j++)
        {
            syn0v_init[i * m_vocabSize + j] = random(-0.1, 0.1)+random(-0.1, 0.1)+random(-0.1, 0.1);
        }
    }

    for(int i = 0; i < layer1_size; i++)
    {
        for(int j = 0; j < m_morphologySize; j++)
        {
            syn0m_init[i * m_morphologySize+ j] = random(-0.1, 0.1)+random(-0.1, 0.1)+random(-0.1, 0.1);
        }
    }

    for(int i = 0; i < layer1_size; i++)
    {
        for(int j = 0; j < layer1_size; j++)
        {
            syn0h_init[i * layer1_size + j] = random(-0.1, 0.1)+random(-0.1, 0.1)+random(-0.1, 0.1);
        }
    }

    syn0v.setMatrix(syn0v_init, layer1_size, m_vocabSize);
    syn0h.setMatrix(syn0h_init, layer1_size, layer1_size);
    syn1v.setMatrix(syn1v_init, m_vocabSize, layer1_size);
    syn1m.setMatrix(syn1m_init, m_morphologySize, layer1_size);

    free(syn1v_init);
    free(syn1m_init);
    free(syn0v_init);
    free(syn0m_init);
    free(syn0h_init);

    if (bptt>0)
    {
        bptt_history_v = std::vector<int>(bptt+bptt_block+10, -1);
        bptt_history_m = std::vector<int>(bptt+bptt_block+10, -1);

        for (size_t i=0; i < bptt + bptt_block; i++)
        {
            Layer newLayer;
            newLayer.setConstant(layer1_size, 0);
            bptt_hidden.push_back(std::move(newLayer));
        }

        bptt_syn0v.setZero(layer1_size, m_vocabSize);
        bptt_syn0m.setZero(layer1_size, m_vocabSize);
        bptt_syn0h.setZero(layer1_size, layer1_size);
    }

    saveWeights();
}

void RnnlmRussianMorphology::netFlush()   //cleans all activations and error vectors
{
    neu0.ac.setConstant(0.1);
    neu0.er.setZero();

    neu1.ac.setZero();
    neu1.er.setZero();

    neu2v.ac.setZero();
    neu2v.er.setZero();

    neu2m.ac.setZero();
    neu2m.er.setZero();

    if (bptt>0)
    {
        for (int a=0; a<bptt+bptt_block; a++) bptt_history_v[a]=0;
        for (int a=0; a<bptt+bptt_block; a++) bptt_history_m[a]=0;
    }
}

void RnnlmRussianMorphology::clearMemory()   //cleans hidden layer activation + bptt history
{
    int a;
    //it really looks like bug: neu1.ac is rewritten at the beginning of computeNet. It is lost unless it is stored in neu0.ac. Setting it to 1 (btw, why 1?!) is meaningless.
    //the same story with Mikolov's code. It explains why key independent does not change results.

    neu1.ac.setConstant(1.0);

    copyHiddenLayerToInput();

    if (bptt>0)
    {
        for (a=1; a<bptt+bptt_block; a++) bptt_history_v[a]=0;
        for (a=1; a<bptt+bptt_block; a++) bptt_history_m[a]=0;
        for (a=bptt+bptt_block-1; a>1; a--)
        {
            bptt_hidden[a].ac.setZero();
            bptt_hidden[a].ac.setZero();
        }
    }
}

void RnnlmRussianMorphology::computeRecurrentLayer_(int i_word, int i_morph)
{
    if(i_word != -1)
    {
        neu1.ac = syn0h * neu0.ac;
        neu1.ac.addMatrixColumn(syn0v, i_word);
        neu1.ac.addMatrixColumn(syn0m, i_morph);
    }
    else
    {
        neu1.ac = syn0h * neu0.ac;
    }
    neu1.ac.logisticActivation();
}

void RnnlmRussianMorphology::computeOutputLayer_()
{
    neu2v.ac = syn1v * neu1.ac;
    neu2v.ac.softmaxActivation();
    neu2m.ac = syn1m * neu1.ac;
    neu2m.ac.softmaxActivation();
}

void RnnlmRussianMorphology::computeErrorOnOutput_(int i_trueWord)
{
    neu2v.fastOutputError(i_trueWord);
    neu2m.fastOutputError(i_trueWord);
}

void RnnlmRussianMorphology::computeErrorOnPrevious_(const Layer& i_nextLayer, Matrix& i_synMat, Layer& i_prevLayer)
{
    i_prevLayer.er = i_synMat.transpose() * i_nextLayer.er;
    i_synMat.transpose();
}

void RnnlmRussianMorphology::applyGradient_(double i_lr, const Vector& i_next, const Vector& i_prev, Matrix& i_mat, double beta)
{
    i_mat.fastGradUpdate(i_lr, i_next, i_prev, beta);
}

void RnnlmRussianMorphology::applyGradient_(double i_lr, const Vector& i_next, int i_column, Matrix& o_mat, double beta)
{
    o_mat.fastGradColumnUpdate(i_lr, i_next, i_column, beta);
}

void RnnlmRussianMorphology::addGradient_(const Matrix& i_update, Matrix& o_mat , double beta2)
{
    o_mat.addExpression(i_update, beta2);
}

void RnnlmRussianMorphology::applyGradient_(const Matrix& i_update, int i_updateColumn, Matrix& io_target, int i_targetColumn, double i_beta)
{
    io_target.addColumnToColumn(i_targetColumn, i_update, i_updateColumn, i_beta);
}

void RnnlmRussianMorphology::incremetLogProbByWordLP_(int word, int morph)
{
    m_logProb += neu2v.ac.elementLog(word);
    m_logProb += neu2m.ac.elementLog(morph);
}

void RnnlmRussianMorphology::updateBptt_(int last_word, int last_morph) //shift memory needed for bptt to next time step
{
    for (int a=bptt+bptt_block-1; a>0; a--)
    {
        bptt_history_v[a]=bptt_history_v[a-1];
        bptt_history_m[a]=bptt_history_m[a-1];
    }
    bptt_history_v[0]=last_word;
    bptt_history_m[0]=last_morph;

    Vector tmp_ac = std::move(bptt_hidden[bptt+bptt_block-1].ac);
    Vector tmp_er = std::move(bptt_hidden[bptt+bptt_block-1].er);

    for (int a=bptt+bptt_block-1; a>0; a--)
    {
          bptt_hidden[a].ac = std::move(bptt_hidden[a-1].ac);
          bptt_hidden[a].er = std::move(bptt_hidden[a-1].er);
    }
    bptt_hidden[0].ac = std::move(tmp_ac);
    bptt_hidden[0].er = std::move(tmp_er);
}

void RnnlmRussianMorphology::makeBptt_(int word, double alpha, double beta, int counter)
{
//    double beta2, beta3;

//    beta2=beta*alpha;
//    beta3=beta2*1;

//    bptt_hidden[0].ac=neu1.ac;
//    bptt_hidden[0].er=neu1.er;

//    if (!((counter%bptt_block==0) || (m_independent && (word==0))))
//        return;

//    for (int step=0; step < bptt+bptt_block-2; step++)
//    {
//        neu1.softmaxErrorActivation();
//        //weight update 1->0
//        int a = bptt_history[step];
//        if (a!=-1)
//        {
//            applyGradient_(alpha,neu1.er,a,bptt_syn0v,0);
//        }

//        computeErrorOnPrevious_(neu1,syn0h,neu0);
//        applyGradient_(alpha, neu1.er, neu0.ac, bptt_syn0h, 0);

//        //propagate error from time T-n to T-n-1
//        neu1.er = neu0.er + bptt_hidden[step+1].er;

//        if (step<bptt+bptt_block-3)
//        {
//            neu1.ac = bptt_hidden[step+1].ac;
//            neu0.ac = bptt_hidden[step+2].ac;
//        }
//    }

//    for (int a = 0; a< bptt + bptt_block; a++)
//    {
//        bptt_hidden[a].er.setZero();
//    }

//    neu1.ac = bptt_hidden[0].ac;		//restore hidden layer after bptt

//    if ((counter%10)==0)
//    {
//        addGradient_(bptt_syn0h,syn0h,beta2);
//        bptt_syn0h.setZero();
//    }
//    else
//    {
//        addGradient_(bptt_syn0h,syn0h,beta2);
//        bptt_syn0h.setZero();
//    }

//    if ((counter%10)==0)
//    {
//        for (int step=0; step<bptt+bptt_block-2; step++)
//            if (bptt_history[step]!=-1)
//            {
//                applyGradient_(bptt_syn0v, bptt_history[step], syn0v, bptt_history[step], beta2);
//                bptt_syn0v.setZeroColumn(bptt_history[step]);
//            }
//    }
//    else
//    {
//        for (int step=0; step<bptt+bptt_block-2; step++) if (bptt_history[step]!=-1)
//        {
//            applyGradient_(bptt_syn0v, bptt_history[step], syn0v, bptt_history[step], 0);
//            bptt_syn0v.setZeroColumn(bptt_history[step]);
//        }
//    }

}

void RnnlmRussianMorphology::readFromFile(FILE *fi, FileTypeEnum filetype)
{
//    goToDelimiter(':', fi);
//    fscanf(fi, "%d", &m_vocabSize);
//    //
//    goToDelimiter(':', fi);
//    fscanf(fi, "%d", &layer1_size);
//    //
//    goToDelimiter(':', fi);
//    fscanf(fi, "%d", &bptt);
//    //
//    goToDelimiter(':', fi);
//    fscanf(fi, "%d", &bptt_block);
//    //
//    goToDelimiter(':', fi);
//    fscanf(fi, "%d", &m_independent);
//    //
//    initNet_();
//    //
//    double d;
//    float fl;
//    if (filetype==TEXT) {
//    goToDelimiter(':', fi);
//    for (int a=0; a<layer1_size; a++) {
//        fscanf(fi, "%lf", &d);
//        neu1.ac[a]=d;
//    }
//    }
//    if (filetype==BINARY) {
//    fgetc(fi);
//    for (int a=0; a<layer1_size; a++) {
//        fread(&fl, 4, 1, fi);
//        neu1.ac[a]=fl;
//    }
//    }

//    neu1.ac.update();

//    double* syn0v_init=(double *)calloc(layer1_size*m_vocabSize, sizeof(double));
//    double* syn0h_init=(double *)calloc(layer1_size*layer1_size, sizeof(double));
//    double* syn1_init = (double *)calloc(layer1_size*(m_vocabSize+1), sizeof(double));

//    //
//    if (filetype==TEXT)
//    {
//        goToDelimiter(':', fi);
//        for (int b=0; b<layer1_size; b++)
//        {
//            for (int a=0; a<m_vocabSize; a++)
//            {
//                fscanf(fi, "%lf", &d);
//                syn0v_init[a+b*m_vocabSize]=d;
//            }
//            for (int a=0; a<layer1_size; a++)
//            {
//                fscanf(fi, "%lf", &d);
//                syn0h_init[a+b*layer1_size]=d;
//            }
//        }
//    }
//    if (filetype==BINARY)
//    {
//        for (int b=0; b<layer1_size; b++)
//        {
//            for (int a=0; a < m_vocabSize; a++)
//            {
//                fread(&fl, 4, 1, fi);
//                syn0v_init[a+b*m_vocabSize]=fl;
//            }
//            for (int a=0; a<layer1_size; a++)
//            {
//                fread(&fl, 4, 1, fi);
//                syn0h_init[a+b*layer1_size]=fl;
//            }
//        }
//    }
//    //
//    if (filetype==TEXT)
//    {
//        goToDelimiter(':', fi);
//        for (int b=0; b<m_vocabSize + 1; b++)
//        {
//            for (int a=0; a<layer1_size; a++)
//            {
//                fscanf(fi, "%lf", &d);
//                syn1_init[a+b*layer1_size]=d;
//            }
//        }
//    }
//    if (filetype==BINARY)
//    {
//        for (int b=0; b<layer1_size + 1; b++)
//        {
//            for (int a=0; a<layer1_size; a++)
//            {
//                fread(&fl, 4, 1, fi);
//                syn1_init[a+b*layer1_size]=fl;
//            }
//        }
//    }

//    syn0v.setMatrix(syn0v_init, layer1_size, m_vocabSize);
//    syn0h.setMatrix(syn0h_init, layer1_size, layer1_size);
//    syn1.setMatrix(syn1_init, m_vocabSize, layer1_size);

//    free(syn0v_init);
//    free(syn0h_init);
//    free(syn1_init);

//    saveWeights();
}

void RnnlmRussianMorphology::writeToFile(FILE *fo, FileTypeEnum filetype)
{
//    float fl = 0;
//    fprintf(fo, "Model\n");
//    fprintf(fo, "vocabulary size: %d\n", m_vocabSize);
//    fprintf(fo, "hidden layer size: %d\n", layer1_size);

//    fprintf(fo, "bptt: %d\n", bptt);
//    fprintf(fo, "bptt block: %d\n", bptt_block);
//    fprintf(fo, "independent sentences mode: %d\n", m_independent);
//    neu1.ac.prepareToSave();
//    if (filetype==TEXT)
//    {
//        fprintf(fo, "\nHidden layer activation:\n");
//        for (int a=0; a<layer1_size; a++) fprintf(fo, "%.4f\n", neu1.ac[a]);
//    }
//    if (filetype==BINARY)
//    {
//        for (int a=0; a<layer1_size; a++)
//        {
//            fl=neu1.ac[a];
//            fwrite(&fl, 4, 1, fo);
//        }
//    }
//    //////////
//    syn0v.prepareToSave();
//    syn0h.prepareToSave();
//    syn1.prepareToSave();

////    syn0v.print();
////    syn0h.print();
////    syn1.print();
////    neu1.ac.print();

//    if (filetype==TEXT)
//    {
//        fprintf(fo, "\nWeights 0->1:\n");
//        for (int b=0; b<layer1_size; b++)
//        {
//            for (int a=0; a<m_vocabSize; a++)
//            {
//                fprintf(fo, "%.4f\n", syn0v.getElement(b,a));
//            }
//            for (int a=0; a<layer1_size; a++)
//            {
//                fprintf(fo, "%.4f\n", syn0h.getElement(b,a));
//            }
//        }
//    }
//    if (filetype==BINARY)
//    {
//        for (int b=0; b<layer1_size; b++)
//        {
//            for (int a=0; a<m_vocabSize; a++)
//            {
//                fl=syn0v.getElement(b,a);
//                fwrite(&fl, 4, 1, fo);
//            }
//            for (int a=0; a<layer1_size; a++)
//            {
//                fl=syn0h.getElement(b,a);
//                fwrite(&fl, 4, 1, fo);
//            }
//        }
//    }
//    /////////
//    if (filetype==TEXT)
//    {
//        fprintf(fo, "\n\nWeights 1->2:\n");
//        for (int b=0; b<m_vocabSize; b++)
//        {
//            for (int a=0; a<layer1_size; a++)
//            {
//                fprintf(fo, "%.4f\n", syn1.getElement(b,a));
//            }
//        }
//        for (int a=0; a<layer1_size; a++)
//        {
//            fprintf(fo, "%.4f\n", 1.0); //we removed classes so for compatibility with the original tool we leave here class weights
//        }

//    }
//    if (filetype==BINARY)
//    {
//        for (int b=0; b<m_vocabSize; b++)
//        {
//            for (int a=0; a<layer1_size; a++)
//            {
//                fl=syn1.getElement(b,a);
//                fwrite(&fl, 4, 1, fo);
//            }
//        }
//        for (int a=0; a<layer1_size; a++)
//        {
//            fl=1.0;
//            fwrite(&fl, 4, 1, fo); //same here
//        }
//    }
//    ////////
}

}