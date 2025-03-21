pipeline{
    agent any
    stages(){
        
        stage('checkout main'){
           
            steps{
                script{
                    echo 'CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC'
                    echo 'CCCCCCC Clonnig !!!! CCCCCCCCC'
                    echo 'CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC'
                    git branch: 'main',
                    url:'https://github.com/egiskorea/XDConvertImage.git',
                    credentialsId: 'github_access_token'
                    
                    def GIT_TAG = sh(script: "git log -1 --pretty=%h", returnStdout: true).trim()
                    env.IMAGE_TAG = "${GIT_TAG}"
                    echo "Generated Image Tag: ${env.IMAGE_TAG}"
                }
                
            }
        }
        
        stage('build'){
            steps{
                  script{
                      sh 'echo building!!!!'
                      sh 'ls ./'
                  }         
                  
            }
        }

        stage('이미지 빌드 및 카카오클라우드 리포지토리 이미지 push!!'){
            agent{
                kubernetes {
                    agentInjection true
                    inheritFrom 'jenkins-agent'
                    cloud 'kakaocloud-dev'
                    yaml """
                            apiVersion: v1
                            kind: Pod
                            metadata:
                              name: kaniko
                              namespace: egis-devops-tools
                            spec:
                              serviceAccountName: egis-admin
                              containers:
                              - name: kaniko
                                image: gcr.io/kaniko-project/executor:debug
                                args: [
                                  "--context=git://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@github.com/egiskorea/XDConvertImage.git#refs/heads/main",
                                  "--destination=digitaltwin-container.kr-central-2.kcr.dev/egis-dev-convert/xdconverter:${env.IMAGE_TAG}"
                                ]
                                tty: false
                                volumeMounts:
                                - name: kaniko-secret
                                  mountPath: /kaniko/.docker
                                - name: kaniko-cache
                                  mountPath: /cache
                              restartPolicy: Always
                              volumes:
                              - name: kaniko-secret
                                secret:
                                  secretName: regcred
                                  items:
                                    - key: .dockerconfigjson
                                      path: config.json
                              - name: kaniko-cache
                                emptyDir: {} 
                        """
                }
            }
            
            steps{
                container('kaniko'){
                    script{
                        try{
                            sh "/kaniko/executor --cleanup  --destination=digitaltwin-container.kr-central-2.kcr.dev/egis-dev-convert/xdconverter:${env.IMAGE_TAG} --context=git://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@github.com/egiskorea/XDConvertImage.git#refs/heads/main"    
                        }catch(Exception e){
                            
                        }
                        
                    }
                    
                }
            }
        }
    }
}