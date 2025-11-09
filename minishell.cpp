#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/types.h>  
#include <map>          
#include <algorithm>    
#include <fcntl.h>      


std::map<pid_t, std::string> g_jobs;


std::vector<std::string> tokenize(const std::string& line);
int execute_command(std::vector<std::string>& tokens);
void shell_loop();
void reap_zombies();
void builtin_jobs();

void handle_redirection(std::vector<std::string>& tokens);
int exec_single_command(std::vector<std::string>& tokens, bool is_background);
int exec_piped_command(std::vector<std::string>& left_cmd, std::vector<std::string>& right_cmd, bool is_background);


std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current_token;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];

        if (isspace(c)) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        } else if (c == '|' || c == '<' || c == '>') {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            tokens.push_back(std::string(1, c));
        } else {
            current_token += c;
        }
    }
    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    return tokens;
}


void reap_zombies() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (g_jobs.count(pid)) {
            std::cout << "\n[Done] " << g_jobs[pid] << std::endl;
            g_jobs.erase(pid);
        }
    }
}


void builtin_jobs() {
    if (g_jobs.empty()) {
        std::cout << "No background jobs." << std::endl;
        return;
    }
    reap_zombies(); 
    for (const auto& pair : g_jobs) {
        std::cout << "[" << pair.first << "] " << pair.second << std::endl;
    }
}


void handle_redirection(std::vector<std::string>& tokens) {
    auto it = tokens.begin();
    while (it != tokens.end()) {
        if (*it == "<") {
            
            it = tokens.erase(it);
            if (it == tokens.end()) {
                std::cerr << "minishell: syntax error near unexpected token `newline'" << std::endl;
                _exit(EXIT_FAILURE);
            }
            
            
            int fd_in = open((*it).c_str(), O_RDONLY);
            if (fd_in == -1) { 
                perror("minishell error"); 
                _exit(EXIT_FAILURE); 
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);

            
            it = tokens.erase(it);
        } else if (*it == ">") {
            
            it = tokens.erase(it);
            if (it == tokens.end()) {
                std::cerr << "minishell: syntax error near unexpected token `newline'" << std::endl;
                _exit(EXIT_FAILURE);
            }

            
            int fd_out = open((*it).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) { 
                perror("minishell error"); 
                _exit(EXIT_FAILURE); 
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);


            it = tokens.erase(it);
        } else {
            
            ++it;
        }
    }
}



int exec_single_command(std::vector<std::string>& tokens, bool is_background) {
    pid_t pid = fork();

    if (pid == 0) {
        
        
        
        handle_redirection(tokens); 

        
        std::vector<char*> args;
        for (const std::string& s : tokens) {
            args.push_back(const_cast<char*>(s.c_str()));
        }
        args.push_back(nullptr); 

        if (args[0] == nullptr) { 
             _exit(EXIT_SUCCESS); 
        }

        
        if (execvp(args[0], args.data()) == -1) {
            perror("minishell error");
        }
        _exit(EXIT_FAILURE); 

    } else if (pid < 0) {
        
        perror("minishell error");
    } else {
        
        if (is_background) {
            std::string command_str;
            for(const auto& s : tokens) { command_str += s + " "; } 
            g_jobs[pid] = command_str;
            std::cout << "[" << pid << "] " << command_str << std::endl;
        } else {
            int status;
            waitpid(pid, &status, 0); 
        }
    }
    return 1;
}


int exec_piped_command(std::vector<std::string>& left_cmd, std::vector<std::string>& right_cmd, bool is_background) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("minishell pipe error");
        return 1;
    }

    pid_t pid1, pid2;

    
    pid1 = fork();
    if (pid1 == 0) {
        
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]);

        
        handle_redirection(left_cmd); 

        
        std::vector<char*> args;
        for (const std::string& s : left_cmd) { args.push_back(const_cast<char*>(s.c_str())); }
        args.push_back(nullptr);

        if (args[0] == nullptr) _exit(EXIT_SUCCESS);
        if (execvp(args[0], args.data()) == -1) {
            perror("minishell error");
        }
        _exit(EXIT_FAILURE);
    }

    
    pid2 = fork();
    if (pid2 == 0) {
        
        close(pipefd[1]); 
        dup2(pipefd[0], STDIN_FILENO); 
        close(pipefd[0]);

        
        handle_redirection(right_cmd);

        
        std::vector<char*> args;
        for (const std::string& s : right_cmd) { args.push_back(const_cast<char*>(s.c_str())); }
        args.push_back(nullptr);

        if (args[0] == nullptr) _exit(EXIT_SUCCESS);
        if (execvp(args[0], args.data()) == -1) {
            perror("minishell error");
        }
        _exit(EXIT_FAILURE);
    }

    
    close(pipefd[0]);
    close(pipefd[1]);

    if (is_background) {
        std::string command_str; 
        for(const auto& s : left_cmd) { command_str += s + " "; }
        command_str += "| ";
        for(const auto& s : right_cmd) { command_str += s + " "; }
        
        g_jobs[pid2] = command_str; 
        std::cout << "[" << pid2 << "] " << command_str << std::endl;
        g_jobs[pid1] = command_str + "(pipe part 1)";

    } else {
        
        waitpid(pid1, nullptr, 0);
        waitpid(pid2, nullptr, 0);
    }

    return 1;
}



int execute_command(std::vector<std::string>& tokens) {
    if (tokens.empty()) return 1;

    bool is_background = false;
    if (tokens.back() == "&") {
        is_background = true;
        tokens.pop_back();
        if (tokens.empty()) return 1;
    }

    if (tokens[0] == "exit") return 0;
    if (tokens[0] == "jobs") {
        builtin_jobs();
        return 1;
    }
    
    auto pipe_it = std::find(tokens.begin(), tokens.end(), "|");

    if (pipe_it != tokens.end()) {
        std::vector<std::string> left_cmd(tokens.begin(), pipe_it);
        std::vector<std::string> right_cmd(pipe_it + 1, tokens.end());
        
        if (left_cmd.empty() || right_cmd.empty()) {
            std::cerr << "minishell: syntax error near unexpected token `|'" << std::endl;
            return 1;
        }
        return exec_piped_command(left_cmd, right_cmd, is_background);
    } else {
        return exec_single_command(tokens, is_background);
    }
}


void shell_loop() {
    std::string line;
    std::vector<std::string> tokens;
    int status = 1;

    while (status) {
        reap_zombies(); 
        std::cout << "minishell> ";
        std::flush(std::cout);

        if (!std::getline(std::cin, line)) {
            break; 
        }

        tokens = tokenize(line);
        status = execute_command(tokens);
    }
    std::cout << "\nExiting minishell. Goodbye!" << std::endl;
}


int main(int argc, char **argv) {
    shell_loop();
    return EXIT_SUCCESS;
}
