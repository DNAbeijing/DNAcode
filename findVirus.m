% ȡ������С��ǰk����Ϊ��ѡ�ĸ�ȾԴ
k = 1;

[nRow, nCol] = size(Coutput);
argminVec = [];

for i = 1:nRow
    [val, id] = sort(Coutput(i,:));
    if min(val) < 0.001
        argminVec = [argminVec; id(1:k)];
    end
end

% �������и�ȾԴ��������״ͼ
figure(1)
hist(argminVec(:), nCol);

% ������С����ķֲ�ͼ
figure(2)
sortMinDist = sort(min(Coutput, [], 2));
plot(sortMinDist(1:end-100));